#pragma once

#include "interpolation_function.hpp"

#include <representations/bembelbots/constants.h>


namespace joints {
namespace details {

template<class J>
class Cubic : public InterpolationFunction<J> {

public:
    Cubic()
        : Cubic(J(), J(), 0, 0) {
    }

    Cubic(const J &from, const J &to, int start, int duration)
            : InterpolationFunction<J>(from, to, start, duration) {
        J n1 = from - to;
        J n2 = to - from;

        supportY1 = from + offset * n1;
        supportY2 = to + offset * n2;
    }

protected:

    void kernel(const J &from, const J &to, J &result, float t) override {
        result = (1 - t) * ( (1 - t) * ( (1 - t) * from + t * supportY1)
                             + t * ( (1 - t) * supportY1 + t * supportY2) )               
              + t *      ( (1 - t) * ( (1 - t) * supportY1 + t * supportY2) 
                             + t * ( (1 - t) * supportY2 + t * to));
    }


private:

    static constexpr float offset = 6.f * M_PI_F / 180.f;

    J supportY1;
    J supportY2;

};
 
} // namespace details
} // namespace joints
