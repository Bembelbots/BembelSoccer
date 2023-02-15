#pragma once

#include "operators.hpp"

#include <framework/util/assert.h>

#include <iostream>


namespace joints {
namespace details {

template<class J>
class InterpolationFunction {

    static_assert(EnableJointOperators<J>, "J must be a joints type.");

public:

    InterpolationFunction() = default;

    InterpolationFunction(const J &from_, const J &to_, int start_, int duration_)
        : from(from_)
        , to(to_)
        , start(start_)
        , duration(duration_) {
    }

    J get(int time) {
        float timePoint = time - start;

        if (timePoint <= 0) {
            return from;
        }

        if (timePoint >= duration) {
            return to;
        }

        kernel(from, to, result, timePoint / duration);

        return result;
    }

    bool done(int time) const {
        return start + duration <= time;
    }

    void setStart(int start) {
        this->start = start;
    }

    void setDuration(int duration) {
        this->duration = duration;
    }

    void setFrom(const J &from) {
        this->from = from;
    }

    void setTo(const J &to) {
        this->to = to;
    }

protected:

    virtual void kernel(const J &, const J &, J &, float) = 0;

private:

    J from;
    J to;

    J result;

    int start = 0; // start time in ms
    float duration = 0;  // duration of interpolation in ms
};

} // namespace details
} // namespace joints
