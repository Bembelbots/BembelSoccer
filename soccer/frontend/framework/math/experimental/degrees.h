#pragma once
#include <iostream>

#include "constants.h"
#include "radians.h"

namespace MathToolbox {
class Radians;

class Degrees {
public:
    static constexpr float max_degrees = 360;
    static constexpr float radtodeg = 180.f / M_PI_F;

    Degrees() = default;

    explicit Degrees(float a);

    explicit Degrees(Radians a);

    operator float() const { return _angle; }

    Degrees normalize() {
        _angle = normalize(_angle);
        return *this;
    }

    static float normalize(float a);

    float pos_normalize() const;

    friend std::ostream &operator<<(std::ostream &os, const Degrees &deg);

private:
    float _angle{0.f};
};
} // namespace MathToolbox
