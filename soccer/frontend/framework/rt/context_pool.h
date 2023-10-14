#pragma once

#include "util/type_info.h"

#include <vector>
#include <memory>

namespace rt {

class ContextPool {

public:
    template<typename T>
    [[ nodiscard ]] T &get() {
        PoolEntry<T> *entry = nullptr;
        for (auto &e : entries) { // cppcheck-suppress useStlAlgorithm
            if (e->type == TypeInfo<T>::id()) {
                entry = static_cast<PoolEntry<T> *>(e.get());
                break;
            }
        }
        if (entry == nullptr) { // cppcheck-suppress nullPointerRedundantCheck
            entry = new PoolEntry<T>{};
            entries.emplace_back(entry);
        }
        return entry->data; // cppcheck-suppress nullPointerRedundantCheck
    }

private:
    struct BasicPoolEntry {
        TypeID type;

        explicit BasicPoolEntry(TypeID t) : type(t) {}
        virtual ~BasicPoolEntry() {}
    };

    template<typename T>
    struct PoolEntry : public BasicPoolEntry {
        T data;

        PoolEntry() : BasicPoolEntry(TypeInfo<T>::id()) {}
    };

    std::vector<std::unique_ptr<BasicPoolEntry>> entries;
};

} // namespace rt
