#pragma once
#include <iostream>

#include "constants.h"
#include "degrees.h"

namespace MathToolbox {
class Degrees;

class Radians {
public:
    static constexpr float max_radians = 2.f * M_PI_F;
    static constexpr float degtorad = M_PI_F / 180.f;

    Radians() = default;

    explicit Radians(float a);

    explicit Radians(Degrees a);

    operator float() const { return _angle; }

    Radians normalize() {
        _angle = normalize(_angle);
        return *this;
    }

    static float normalize(float a);

    float pos_normalize() const;

    friend std::ostream &operator<<(std::ostream &os, const Radians &rad);

private:
    float _angle{0.f};
};
} // namespace MathToolbox
