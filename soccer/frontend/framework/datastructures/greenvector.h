#pragma once
#include <vector>
#include <functional>

template<typename T>
class GreenVector {
private:
    using storage_t = typename std::vector<T>;
    using iterator = typename storage_t::iterator;
    using const_iterator = typename storage_t::const_iterator;
    using size_type = typename storage_t::size_type;
    using index_type = size_type;

    std::vector<index_type> recycled;
    storage_t storage;

public:

    T& insert(std::function<T(index_type)> create, std::function<void (index_type, T&)> recycle) {
        if(!recycled.empty()) {
            auto idx = recycled.back();
            recycle(idx, storage[idx]);
            recycled.pop_back();
            return storage[idx];
        }
        auto idx = storage.size();
        storage.push_back(create(idx));
        return storage.back();
    }

    void recycle(index_type i) {
        if(i == storage.size() - 1) {
            storage.pop_back();
        } else {
            recycled.push_back(i);
        }
    }

    void recycle(const iterator &it) {
        recycle(std::distance(storage.begin(), it));
    }

    T& operator [](index_type idx) {
        return storage[idx];
    }

    const T& operator [](index_type idx) const {
        return storage[idx];
    }

    iterator begin() {
        return storage.begin();
    }

    iterator end() {
        return storage.end();
    }

    const_iterator begin() const {
        return storage.begin();
    }

    const_iterator end() const {
        return storage.end();
    }

    size_type size() const {
        return storage.size();
    }
};