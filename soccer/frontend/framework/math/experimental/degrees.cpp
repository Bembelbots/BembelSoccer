#include "degrees.h"
#include "math.h"

namespace MathToolbox {

Degrees::Degrees(float a) {
    _angle = normalize(a);
}

Degrees::Degrees(Radians a) {
    _angle = normalize(a * radtodeg);
}

float Degrees::normalize(float a) {
    return remainder(a, max_degrees);
}

float Degrees::pos_normalize() const {
    return (_angle < 0) ? _angle + max_degrees : _angle;
}

std::ostream &operator<<(std::ostream &os, const Degrees &deg) {
    os << deg._angle << "°";
    return os;
}

} // namespace MathToolbox
