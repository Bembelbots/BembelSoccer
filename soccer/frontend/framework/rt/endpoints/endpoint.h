#pragma once

#include "../flags.h"
#include "../channels.h"
#include "../../util/assert.h"

namespace rt {

class Linker;

template<typename T>
class Endpoint {

public:
    using QualifiedType = T;
    using PureType = typename std::remove_const_t<T>;

    QualifiedType &get() {
        jsassert(data != nullptr);
        return *data;
    }

    const PureType &get() const {
        jsassert(data != nullptr);
        return *data;
    }

    inline QualifiedType &operator*() { return get(); }
    inline const PureType &operator*() const { return get(); }

    inline QualifiedType *operator->() { return data; }
    inline const PureType *operator->() const { return data; }

    inline operator QualifiedType &() { return get(); }
    inline operator const PureType &() const { return get(); }

    inline operator QualifiedType *() { return data; }
    inline operator const PureType *() const { return data; }

protected:
    friend class Linker;
    PureType *data = nullptr;
};

} // namespace rt
