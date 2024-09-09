#pragma once

#include "lola_names_generated.h"
#include "mask.h"

#include <cmath>
#include <type_traits>

#include <representations/flatbuffers/types/lola_names.h>

namespace joints {
namespace details {

template<class J>
static constexpr bool EnableJointOperators = false;

#define JOINTS_ENABLE_OPERATORS(JointClass) \
    template<Mask JB> \
    static constexpr bool EnableJointOperators<JointClass<JB>> = true;

template<class Joint, class T = Joint>
using EnableJoint = std::enable_if_t<EnableJointOperators<Joint>, T>;

template<Mask M, typename LoopBody>
inline void each(LoopBody body) {
    for (JointNames i : bbapi::EnumValuesJointNames()) {
        if (any(idToMask(i) & M)) {
            body(i);
        }
    }
}

template<class J1, class J2>
EnableJoint<J1> &operator+=(J1 &j1, const J2 &j2) {
    j2.each([&](JointNames i) {
        if (j1.contains(i))
            j1[i] += j2[i];
    });
    return j1;
}

template<class J1, class J2>
EnableJoint<J1> &operator-=(J1 &j1, const J2 &j2) {
    j2.each([&](JointNames i) {
        if (j1.contains(i))
            j1[i] -= j2[i];
    });
    return j1;
}

template<class J>
EnableJoint<J> &operator+=(J &j, float s) {
    J::each([&](JointNames i) { j[i] += s; });
    return j;
}

template<class J>
EnableJoint<J> &operator-=(J &j, float s) {
    J::each([&](JointNames i) { j[i] -= s; });
    return j;
}

template<class J>
EnableJoint<J> &operator*=(J &j, float s) {
    J::each([&](JointNames i) { j[i] *= s; });
    return j;
}

template<class J>
EnableJoint<J> &operator/=(J &j, float s) {
    J::each([&](JointNames i) { j[i] /= s; });
    return j;
}

template<class J>
EnableJoint<J> operator+(const J &j1, const J &j2) {
    J res = j1;
    return res += j2;
}

template<class J>
EnableJoint<J> operator-(const J &j1, const J &j2) {
    J res = j1;
    return res -= j2;
}

template<class J>
EnableJoint<J> operator+(const J &j1, float s) {
    J res = j1;
    return res += s;
}

template<class J>
EnableJoint<J> operator-(const J &j1, float s) {
    J res = j1;
    return res -= s;
}

template<class J>
EnableJoint<J> operator*(float s, const J &j) {
    J res = j;
    return res *= s;
}

template<class J>
EnableJoint<J> operator/(const J &j, float s) {
    J res = j;
    return res /= s;
}

template<class J>
EnableJoint<J, bool> operator==(const J &j1, const J &j2) {
    bool eq = true;
    J::each([&](JointNames i) { eq &= (j1[i] == j2[i]); });
    return eq;
}

template<class J>
EnableJoint<J, bool> operator!=(const J &j1, const J &j2) {
    return !(j1 == j2);
}

template<class J>
EnableJoint<J, bool> operator<(const J &j1, const J &j2) {
    bool cond = true;
    J::each([&](JointNames i) { cond &= (j1[i] < j2[i]); });
    return cond;
}

template<class J>
EnableJoint<J, bool> operator<=(const J &j1, const J &j2) {
    bool cond = true;
    J::each([&](JointNames i) { cond &= (j1[i] <= j2[i]); });
    return cond;
}

template<class J>
EnableJoint<J, bool> operator>(const J &j1, const J &j2) {
    return !(j1 <= j2);
}

template<class J>
EnableJoint<J, bool> operator>=(const J &j1, const J &j2) {
    return !(j1 < j2);
}

template<class J>
EnableJoint<J, bool> operator<(const J &j, float f) {
    bool cond = true;
    J::each([&](JointNames i) { cond &= (j[i] < f); });
    return cond;
}

template<class J>
EnableJoint<J, bool> operator<=(const J &j, float f) {
    bool cond = true;
    J::each([&](JointNames i) { cond &= (j[i] < f); });
    return cond;
}

template<class J>
EnableJoint<J, bool> operator>(const J &j, float f) {
    return !(j <= f);
}

template<class J>
EnableJoint<J, bool> operator>=(const J &j, float f) {
    return !(j < f);
}

template<class J>
EnableJoint<J, bool> feq(const J &j1, const J &j2, float epsilon = 1e-5) {
    bool eq = true;
    J::each([&](JointNames i) { eq &= std::abs(j1[i] - j2[i]) < epsilon; });
    return eq;
}

} // namespace details
} // namespace joints
