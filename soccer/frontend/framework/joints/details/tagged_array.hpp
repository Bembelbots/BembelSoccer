#pragma once

#include <array>
#include <cstring>
#include <type_traits>


namespace joints {
namespace details {

template<typename T, size_t N, typename Tag>
class TaggedArray {

    static_assert(std::is_trivially_copyable<T>::value, 
            "T must be trivially copyable");

public:

    using ArrayType = std::array<T, N>;

          ArrayType &get()       { return arr; }
    const ArrayType &get() const { return arr; }

          T &operator[](size_t ind)       { return arr[ind]; }
    const T &operator[](size_t ind) const { return arr[ind]; }


    void fill(const T *src) __attribute__((nonnull)) {
        std::memcpy(arr.data(), src, N * sizeof(T));
    }

    void spill(T *dst) const __attribute__((nonnull)) {
        std::memcpy(dst, arr.data(), N * sizeof(T));
    }

private:

    ArrayType arr;
    
};

} // namespace details
} // namespace joints
