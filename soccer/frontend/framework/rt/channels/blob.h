#pragma once

#include "../message_utils.h"
#include "../util/ring_buffer.h"

#include <mutex>

namespace rt {

template<typename T, typename = typename std::enable_if_t<is_blob_v<T>>>
class BlobChannel {

public:
    using DataT = typename T::DataT;
    using PayloadT = typename T::PayloadT;

    static constexpr bool HasPayload = T::HasPayload;
    static constexpr size_t Alignment = 64;

    BlobChannel() : size(), buffer(Alignment, size.flat()) {
        addEntry();
        addEntry();
    }

    int addListener() {
        readers.emplace_back();
        addEntry();
        return readers.size() - 1;
    }

    void addWriter(const Dim &size) {
        std::lock_guard lock{mtx};

        this->size = size;
        buffer.setSize(size.flat());
        addListener();
    }

    void startWriting(T &blob) {
        std::lock_guard lock{mtx};
        blob.data = buffer.startWriting();
        blob.size = size;
    }

    void finishWriting(T &blob) {
        std::lock_guard lock{mtx};

        setPayload(buffer.getWriteBuf(), blob.info);
        buffer.finishWriting();
        blob.data = nullptr;
        blob.size = Dim{};
        tick++;
    }

    bool hasNewData(int id) const { return assureReader(id).tick < tick; }

    void startReading(int id, T &blob, bool requiresNewData = false) {
        std::lock_guard lock{mtx};

        jsassert(requiresNewData ? hasNewData(id) : true);
        auto &ctx = assureReader(id);
        ctx.bufId = buffer.addReader();
        ctx.tick = tick;
        blob.data = buffer.data(ctx.bufId);
        blob.size = size;
        blob.info = getPayload(ctx.bufId);
    }

    void stopReading(int id, T &blob) {
        std::lock_guard lock{mtx};

        buffer.removeReader(assureReader(id).bufId);
        blob.data = nullptr;
        blob.size = Dim{};
        blob.info = PayloadT{};
    }

private:
    struct ReaderCtx {
        size_t tick = 0;
        size_t bufId = RingBuffer<DataT>::NO_REF;
    };

    std::mutex mtx;
    size_t tick = 0;
    Dim size;
    RingBuffer<DataT> buffer;
    std::vector<PayloadT> payload;

    std::vector<ReaderCtx> readers;

    PayloadT getPayload(size_t id) const {
        if constexpr (HasPayload) {
            return payload.at(id);
        } else {
            return NoPayload{};
        }
    }

    void setPayload(size_t id, const PayloadT &p) {
        if constexpr (HasPayload) {
            payload.at(id) = p;
        }
    }

    void addEntry() {
        if constexpr (HasPayload) {
            payload.emplace_back();
        }
        buffer.addEntry();
    }

    ReaderCtx &assureReader(int id) {
        jsassert(id > -1 && size_t(id) < readers.size());
        return readers[id];
    }

    const ReaderCtx &assureReader(int id) const {
        jsassert(id > -1 && size_t(id) < readers.size());
        return readers[id];
    }
};

} // namespace rt
