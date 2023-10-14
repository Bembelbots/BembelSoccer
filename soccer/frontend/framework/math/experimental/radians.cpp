#include "radians.h"
#include "math.h"

namespace MathToolbox {
Radians::Radians(float a) {
    _angle = normalize(a);
}

Radians::Radians(Degrees a) {
    _angle = normalize(a * degtorad);
}

float Radians::normalize(float a) {
    return remainder(a, max_radians);
}

float Radians::pos_normalize() const {
    return (_angle < 0) ? _angle + max_radians : _angle;
}

std::ostream &operator<<(std::ostream &os, const Radians &rad) {
    os << rad._angle << " rad";
    return os;
}
} // namespace MathToolbox
