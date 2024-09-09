#pragma once

#include <algorithm>
#include <cmath>

inline bool floatEQ(const float x, const float y) {
    return std::abs(x - y) < 0.00001f;
}

// test, and 'true'  if value is 'between' two other values
template<typename T>
inline bool between(const T &val, const T &lower, const T &upper) {
    return (val >= lower && val <= upper);
}

// scale 'src_val' to target codomain [from ... to] default: [0 ... 1]
// using source values 'src_max', 'src_min' (safe -> check max/min)
template<typename T, typename S>
inline const T scale(
        const S &x, const S &src_min, const S &src_max, const T &targ_from = 0.0f, const T &targ_to = 1.0f) {

    // shift, norm to [0..1], divide with own range
    // multiply with target range, shift to legal range
    const T out = (float(x - src_min) / float(src_max - src_min)) * (targ_to - targ_from) + targ_from;

    // make sure the boundaries are not violated
    return std::clamp(out, targ_from, targ_to);
}
