#pragma once

#include "../util/assert.h"

#include <atomic>
#include <cstdlib>
#include <limits>
#include <utility>
#include <vector>

namespace rt {

template<typename T>
class RingBuffer {

public:
    static constexpr size_t NO_REF = std::numeric_limits<size_t>::max();

    RingBuffer(size_t alignment, size_t size) : alignment(alignment), size(size) {}

    T *startWriting() {
        jsassert(writeBuf == NO_REF); // Can't write on more than one buffer at once
        for (size_t i = 0; i < cells.size(); i++) {
            if (tryLockForWriting(i)) {
                writeBuf = i;
                return cells[i].data;
            }
        }

        // FIXME: Raise error here. There must always be at least one free cell
        // for writing
        return nullptr;
    }

    void finishWriting() {
        jsassert(writeBuf != NO_REF);
        bool ok = tryLockForReading(writeBuf);
        jsassert(ok);
        newestData = std::exchange(writeBuf, NO_REF);
    }

    size_t addReader() {
        jsassert(newestData != NO_REF);
        int r = cells[newestData].refCount.fetch_add(1);
        jsassert(r >= 0);
        return newestData;
    }

    void removeReader(size_t id) {
        jsassert(id != NO_REF);
        int r = cells.at(id).refCount.fetch_sub(1);
        jsassert(r >= 0);
    }

    void setSize(size_t newSize) {
        size = newSize;
        size_t numCells = cells.size();
        cells.clear();
        for (size_t i = 0; i < numCells; i++) {
            addEntry();
        }
    }

    void addEntry() { cells.emplace_back(alignment, size); }

    size_t getWriteBuf() const { return writeBuf; }
    T *data(size_t id) { return cells[id].data; }

    bool hasNewData(size_t id) const { return id != newestData; }

private:
    struct Cell {
        T *data;
        std::atomic<int> refCount;

        Cell() : data(nullptr) { refCount.store(0); }

        Cell(size_t alignment, size_t size) {
            // std::aligned_alloc fails on V5 (gcc7)
            //data = static_cast<T *>(std::aligned_alloc(alignment, nextMultipleOf(sizeof(T) * size, alignment)));
            int ret = posix_memalign(reinterpret_cast<void **>(&data), alignment, nextMultipleOf(sizeof(T) * size, alignment));
            jsassert(ret == 0) << "posix_memalign() failed";
            refCount.store(0);
        }

        Cell(const Cell &) = delete;

        Cell(Cell &&other) : refCount(other.refCount.load()) {
            data = other.data;
            other.data = nullptr;
            other.refCount = 0;
        }

        ~Cell() { std::free(data); }

        static size_t nextMultipleOf(size_t n, size_t x) { return n - (n % x) + x; }
    };

    std::vector<Cell> cells;

    size_t writeBuf = NO_REF;
    size_t newestData = NO_REF;

    size_t alignment = 0;
    size_t size = 0;

    bool tryLockForWriting(size_t id) {
        int expected = 0;
        return cells[id].refCount.compare_exchange_weak(expected, -1);
    }

    bool tryLockForReading(size_t id) {
        int expected = -1;
        return cells[id].refCount.compare_exchange_weak(expected, 0);
    }
};

} // namespace rt
