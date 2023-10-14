#include "angle.h"

namespace MathToolbox {
bool operator>(const Angle &angle, const Degrees &other) {
    return angle.deg > (float)other;
}

bool operator>=(const Angle &angle, const Degrees &other) {
    return angle.deg >= (float)other;
}

bool operator<(const Angle &angle, const Degrees &other) {
    return angle.deg < (float)other;
}

bool operator<=(const Angle &angle, const Degrees &other) {
    return angle.deg <= (float)other;
}

bool operator>(const Degrees &other, const Angle &angle) {
    return (float)other > angle.deg;
}

bool operator>=(const Degrees &other, const Angle &angle) {
    return (float)other >= angle.deg;
}

bool operator<(const Degrees &other, const Angle &angle) {
    return (float)other < angle.deg;
}

bool operator<=(const Degrees &other, const Angle &angle) {
    return (float)other <= angle.deg;
}

bool operator>(const Angle &angle, const Radians &other) {
    return (float)angle > (float)other;
}

bool operator>=(const Angle &angle, const Radians &other) {
    return (float)angle >= (float)other;
}

bool operator<(const Angle &angle, const Radians &other) {
    return (float)angle < (float)other;
}

bool operator<=(const Angle &angle, const Radians &other) {
    return (float)angle <= (float)other;
}

bool operator>(const Radians &other, const Angle &angle) {
    return (float)other > (float)angle;
}

bool operator>=(const Radians &other, const Angle &angle) {
    return (float)other >= (float)angle;
}

bool operator<(const Radians &other, const Angle &angle) {
    return (float)other < (float)angle;
}

bool operator<=(const Radians &other, const Angle &angle) {
    return (float)other <= (float)angle;
}

bool operator>(const Angle &angle, const Angle &other) {
    return (float)angle > (float)other;
}

bool operator>=(const Angle &angle, const Angle &other) {
    return (float)angle >= (float)other;
}

bool operator<(const Angle &angle, const Angle &other) {
    return (float)angle < (float)other;
}

bool operator<=(const Angle &angle, const Angle &other) {
    return (float)angle <= (float)other;
}

std::ostream &operator<<(std::ostream &os, const Angle &angle) {
    os << "[ " << angle.deg << ", " << angle.rad << " ]";
    return os;
}

} // namespace MathToolbox