#pragma once

#include "../util/assert.h"

#include <gsl/span>
#include <type_traits>

namespace rt {

template<typename Command, typename Group>
struct is_command : std::false_type {};

template<typename Command, typename Group>
inline constexpr bool is_command_v = is_command<Command, Group>::value;

#define RT_REGISTER_COMMAND(type, group) \
    template<> \
    struct rt::is_command<type, group> : std::true_type {}

struct Dim {
    size_t x, y, z;

    Dim() : Dim(1, 1, 1) {}
    explicit Dim(size_t x_, size_t y_ = 1, size_t z_ = 1) : x(x_), y(y_), z(z_) {}

    size_t flat() const { return x * y * z; }

    bool operator==(const Dim &other) const { return x == other.x && y == other.y && z == other.z; }
};

// Used to check if a message is a blob
// Shouldn't be inherited by anyone else...
struct BlobBase {};

// Special type to indicate that a blob message doesn't have a payload
struct NoPayload {};

template<typename T>
struct is_blob : std::is_base_of<BlobBase, T> {};

template<typename T>
inline constexpr bool is_blob_v = is_blob<T>::value;

template<typename T, typename P = NoPayload>
struct Blob : public BlobBase {
    static constexpr bool HasPayload = not std::is_same_v<P, NoPayload>;
    using PayloadT = P;
    using DataT = T;

    DataT *data;
    Dim size;
    PayloadT info;
};

template<typename T>
struct Squash {
    void operator()(T &base, const T &newer) const { base = newer; }
};

template<typename T>
void squash(T &base, const T &newer) {
    Squash<T> impl{};
    return impl(base, newer);
}

template<typename T>
T squashAll(gsl::span<T> vals) {
    jsassert(not vals.empty());
    T ret = vals[0];
    for (size_t i = 1; i < vals.size(); i++) {
        squash(ret, vals[i]);
    }
    return ret;
}

} // namespace rt
