#pragma once

#include <cstdint>

#include "joints.h"
#include "mask.h"
#include "operators.hpp"

#include <representations/flatbuffers/types/sensors.h>

namespace joints {
namespace details {

template<Mask JB>
class JointsBase {

public:
    using BasicType = bbipc::JointArray;

    static constexpr Mask mask = JB;

    static constexpr inline bool contains(size_t i) {
        if constexpr (mask == Mask::All) {
            return i < LOLA_NUMBER_OF_JOINTS;
        } else {
            return i < LOLA_NUMBER_OF_JOINTS && any(intToMask(i) & mask);
        }
    }

    static constexpr inline bool contains(JointNames i) {
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

    inline float tryAt(JointNames i, float fallback) const { return contains(i) ? data[static_cast<int>(i)] : fallback; }

    inline float &at(size_t i) {
        jsassert(contains(i)) << "  i=" << i;
        return data[i];
    }

    inline const float &at(size_t i) const {
        jsassert(contains(i)) << "  i=" << i;
        return data[i];
    }

    inline float &at(JointNames i) {
        jsassert(contains(i)) << "  i=" << i;
        return data[static_cast<int>(i)];
    }

    inline const float &at(JointNames i) const {
        jsassert(contains(i)) << "  i=" << i;
        return data[static_cast<int>(i)];
    }

    inline float &operator[](size_t i) { return at(i); }

    inline const float &operator[](size_t i) const { return at(i); }

    inline float &operator[](JointNames i) { return at(i); }

    inline const float &operator[](JointNames i) const { return at(i); }

protected:
    BasicType data;

    void read(const BasicType &src) {
        details::each<Mask::All>([&](JointNames i) { data[static_cast<int>(i)] = src[static_cast<int>(i)]; });
    }

    void write(BasicType &dst) const {
        each([&](JointNames i) { dst[static_cast<int>(i)] = data[static_cast<int>(i)]; });
    }
};

template<Mask JB>
std::ostream &operator<<(std::ostream &os, const JointsBase<JB> &j) {
    JointsBase<JB>::each([&](JointNames i) { os << bbapi::EnumNamesJointNames()[static_cast<int>(i)] << " = " << j[i] << std::endl; });
    return os;
}

} // namespace details
} // namespace joints
