#pragma once

#include "interpolation_function.hpp"


namespace joints {
namespace details {

template<class J>
class Linear : public InterpolationFunction<J> {

public:

    Linear() = default;

    Linear(const J &from, const J &to, int start, int duration)
            : InterpolationFunction<J>(from, to, start, duration) {
    }

protected:

    void kernel(const J &from, const J &to, J &result, float t) override {
        // Use identity (1-t)x + ty = x + t(y-x) to prevent mallocs
        result = to;
        result -= from;
        result *= t;
        result += from;
    }

};

} // namespace details
} // namespace joints

// vim: set ts=4 sw=4 sts=4 expandtab:
