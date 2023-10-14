#include "coord.h"
#include "utils.h"

#include <algorithm>
#include <cmath>
#include <ostream>

Coord::Coord() : x(0.0f), y(0.0f) {
}

Coord::Coord(const float &x, const float &y) : x(x), y(y) {
}

Coord::Coord(const int &x, const int &y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {
}

Coord::Coord(const Angle &theta) : Coord(1.f, 0.f) {
    *this = rotate(theta);
}

float Coord::norm2() const {
    return std::sqrt(x * x + y * y);
}

float Coord::dot(const Coord &other) const {
    return (x * other.x + y * other.y);
}

Coord Coord::normalized() const {
    return Coord(x / this->norm2(), y / this->norm2());
}

Coord Coord::clamp(float maxVal) const {
    float maxVal_tmp = std::max(std::abs(x), std::abs(y));
    float factor = (maxVal_tmp != 0) ? maxVal / maxVal_tmp : 0.0f;
    if (std::abs(factor) < 1.0f) {
        return Coord(x * std::abs(factor), y * std::abs(factor));
    } else {
        return Coord(x, y);
    }
}

Coord Coord::scale_maxValue_to(float maxVal) const {
    float maxVal_tmp = std::max(std::abs(x), std::abs(y));
    float factor = (maxVal_tmp != 0) ? maxVal / maxVal_tmp : 0.0f;
    return {x * std::abs(factor), y * std::abs(factor)};
}

Angle Coord::direction() const {
    return Rad{atan2f(y, x)};
}

float Coord::dist(const Coord &target) const {
    float dx = target.x - x;
    float dy = target.y - y;
    return hypotf(dx, dy);
}

float Coord::dist() const {
    return hypotf(x, y);
}

Angle Coord::angle() const {
    return direction();
}

Coord Coord::rotate(const Angle &angle) const {
    float alpha = angle.rad();
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);

    Coord out(*this);
    float _x = out.x * cosalpha - out.y * sinalpha;
    out.y = out.x * sinalpha + out.y * cosalpha;
    out.x = _x;

    return out;
}

Coord &Coord::add(const Coord &other) {
    x += other.x;
    y += other.y;
    return *this;
}

Coord &Coord::sub(const Coord &other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Coord Coord::operator-(const Coord &other) const {
    return {x - other.x, y - other.y};
}

Coord Coord::operator+(const Coord &other) const {
    return {x + other.x, y + other.y};
}

Coord Coord::operator*(const Coord &other) const {
    return {x * other.x, y * other.y};
}

Coord operator*(float scalar, const Coord &other) {
    return {scalar * other.x, scalar * other.y};
}

Coord operator*(const Coord &other, float scalar) {
    return {other.x * scalar, other.y * scalar};
}

Coord operator/(float scalar, const Coord &other) {
    return {scalar / other.x, scalar / other.y};
}

Coord &Coord::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

bool Coord::operator==(const Coord &other) const {
    return floatEQ(other.x, x) && floatEQ(other.y, y);
}

bool Coord::operator!=(const Coord &other) const {
    return !(*this == other);
}

Coord operator/(const Coord &other, float scalar) {
    return {other.x / scalar, other.y / scalar};
}

Coord &Coord::add(const Angle &dir, const float &dist) {
    x += dist * cosf(dir.rad());
    y += dist * sinf(dir.rad());
    return *this;
}

Coord Coord::closestPointOnLine(Coord start, Coord end, bool onLine) const {
    //berrechne Faktor der Geraden, mit der der Punkt auf der Geraden bestimmt werden kann, der die kürzeste Entfernung zu this hat
    //skalarprodukt: (gerade -Punkt) * (end-start) =0
    if (((end.x - start.x) + (end.y - start.y)) == 0) {
        return Coord(start.x, start.y);
    }

    float dxSquared = end.x - start.x;
    dxSquared *= dxSquared;

    float dySquared = end.y - start.y;
    dySquared *= dySquared;
    float _scale = -(((start.x - x) * (end.x - start.x)) + ((start.y - y) * (end.y - start.y))) / (dxSquared + dySquared);
    if (onLine) {
        _scale = std::max(0.0f, std::min(1.f, _scale)); //point shoudl be on line
    }
    return Coord((start.x + _scale * (end.x - start.x)), start.y + _scale * (end.y - start.y));
};

std::ostream &operator<<(std::ostream &s, const Coord &obj) {
    return s << obj.x << "," << obj.y;
}
