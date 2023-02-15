#pragma once

#include "../util/assert.h"

#include <cstddef>

namespace rt {
/**
 * std::vector-like container that doesn't need to grow in size dynamically
 * but the size isn't known at compile time either.
 * Note that calling resize() will delete all items in the vector and construct new ones.
 * This is useful for storing types that don't have a move- or copy-operator
 * like std::mutex or std::condition_variable.
 */
template<typename T>
class StaticVector {

public:
    StaticVector() = default;

    ~StaticVector() { clear(); }

    void clear() {
        delete[] _data;
        _size = 0;
    }

    void resize(size_t n) {
        clear();
        _data = new T[n]{};
        _size = n;
    }

    T &at(size_t i) {
        jsassert(i < _size);
        return _data[i];
    }

    const T &at(size_t i) const {
        jsassert(i < _size);
        return _data[i];
    }

    inline size_t size() const { return _size; }
    inline T *data() { return _data; }
    inline const T *data() const { return _data; }

    T &operator[](size_t i) { return at(i); }
    const T &operator[](size_t i) const { return at(i); }

private:
    T *_data = nullptr;
    size_t _size = 0;
};

} // namespace rt
