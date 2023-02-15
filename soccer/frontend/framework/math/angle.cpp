#include "angle.h"
#include "constants.h"
#include "utils.h"

#include "../util/assert.h"

#include <cmath>
#include <ostream>

float Angle::normalize(const float &rad) {
    return remainderf(rad, 2.f * M_PI_F);
}

bool Angle::isNormalized(float rad) {
    return between(rad, -M_PI_F, M_PI_F);
}

float Angle::posNormalized() const {
    return (_rad < 0.f) ? _rad + 2.f * M_PI_F : _rad;
}

Angle Angle::operator-(const Angle &other) const {
    return Rad{this->_rad - other._rad};
}

Angle Angle::operator+(const Angle &other) const {
    return Rad{this->_rad + other._rad};
}

Angle Angle::operator*(const Angle &other) const {
    return Rad{this->_rad * other._rad};
}

Angle Angle::operator*(float scale) const {
    return Rad{this->_rad * scale};
}

Angle Angle::operator/(float scale) const {
    return Rad{this->_rad / scale};
}


Angle Angle::operator-() const {
    return Rad{-_rad};
}

Angle &Angle::operator+=(const Angle &other) {
    *this = *this + other;
    return *this;
}

Angle &Angle::operator-=(const Angle &other) {
    *this = *this - other;
    return *this;
}

Angle &Angle::operator*=(float scale) {
    *this = *this * scale;
    return *this;
}
    
bool Angle::operator<(const Angle &other) const {
    return _rad < other._rad;
}

bool Angle::operator>(const Angle &other) const {
    return _rad > other._rad;

}

bool Angle::operator==(const Angle &other) const {
    return fabsf(_rad - other._rad) < 0.0001f;
}

Angle &Angle::set(const Angle &obj) {
    return *this = obj;
}

Angle &Angle::add(const Angle &other) {
    return *this += other;
}

Angle &Angle::sub(const Angle &other) {
    return *this -= other;
}

Angle Angle::clamp(float maxVal) const {
    float factor;
    factor = (_rad != 0) ? maxVal / _rad : 0.0f;
    if (std::abs(factor) < 1.0f) {
        return Rad{_rad * std::abs(factor)};
    } else {
        return Rad{_rad};
    }
}

Angle Angle::dist(const Angle &ang) const {
    return *this - ang;
}

Angle Angle::merge(const Angle &other, const float &my_weight, const float &other_weight) {
    jsassert(isNormalized(_rad));
    jsassert(isNormalized(other._rad));
    float a1 = _rad;
    float a2 = other._rad;
    float diff = a2 - a1;

    static const float pi2 = M_PI_F * 2.f;
    if (diff > M_PI_F) {
        a1 += pi2;
    } else if (diff < -M_PI_F) {
        a1 -= pi2;
    }

    float m = ((my_weight * a1) + (other_weight * a2));
    m /= (my_weight + other_weight);

    return Rad{m};
}

std::ostream &operator<<(std::ostream &s, const Angle &obj) {
    return s << obj.rad() << "(rad)";
}
