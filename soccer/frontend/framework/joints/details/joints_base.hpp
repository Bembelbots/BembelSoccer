#pragma once

#include "joints.h"
#include "mask.h"
#include "operators.hpp"

#include <cstdint>

namespace joints {
namespace details {

template<Mask JB>
class JointsBase {

public:
    using BasicType = std::array<float, NR_OF_JOINTS>;

    static constexpr Mask mask = JB;

    static constexpr inline bool contains(size_t i) {
        if constexpr (mask == Mask::All) {
            return i < NR_OF_JOINTS;
        } else {
            return i < NR_OF_JOINTS && any(intToMask(i) & mask);
        }
    }

    static constexpr inline bool contains(joint_id i) {
        if constexpr (mask == Mask::All) {
            return true;
        } else {
            return any(idToMask(i) & mask);
        }
    }

    template<typename LoopBody>
    static inline void each(LoopBody body) {
        details::each<mask, LoopBody>(body);
    }

    JointsBase() { fill(0); }

    explicit JointsBase(const Joints &j) : JointsBase(j.arr()) {}

    explicit JointsBase(const BasicType &init) : data(init) {}

    void fill(float s) { data.fill(s); }

    constexpr inline size_t size() const { return data.size(); }

    inline float tryAt(size_t i, float fallback) const { return contains(i) ? data[i] : fallback; }

    inline float tryAt(joint_id i, float fallback) const { return contains(i) ? data[i] : fallback; }

    inline float &at(size_t i) {
        jsassert(contains(i));
        return data[i];
    }

    inline const float &at(size_t i) const {
        jsassert(contains(i));
        return data[i];
    }

    inline float &at(joint_id i) {
        jsassert(contains(i));
        return data[i];
    }

    inline const float &at(joint_id i) const {
        jsassert(contains(i));
        return data[i];
    }

    inline float &operator[](size_t i) { return at(i); }

    inline const float &operator[](size_t i) const { return at(i); }

    inline float &operator[](joint_id i) { return at(i); }

    inline const float &operator[](joint_id i) const { return at(i); }

protected:
    std::array<float, NR_OF_JOINTS> data;

    void read(const float *src, const std::array<size_t, NR_OF_JOINTS> &ind) {
        details::each<Mask::All>([&](joint_id i) { data[i] = src[ind[i]]; });
    }

    void write(float *dst, const std::array<size_t, NR_OF_JOINTS> &ind) const {
        each([&](joint_id i) { dst[ind[i]] = data[i]; });
    }
};

template<Mask JB>
std::ostream &operator<<(std::ostream &os, const JointsBase<JB> &j) {
    JointsBase<JB>::each([&](joint_id i) { os << kin::IdNameTable.at(i) << " = " << j[i] << std::endl; });
    return os;
}

} // namespace details
} // namespace joints
