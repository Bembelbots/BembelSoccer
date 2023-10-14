#include "coords.h"

#include "../../../frontend/src/core/util/constants.h"

#include <math.h>
#include <vector>
#include <iostream>

namespace MathToolbox {

Coord::Coord() : x(0.0f), y(0.0f) {
}

Coord::Coord(const float &x, const float &y) : x(x), y(y) {
}

Coord::Coord(const int &x, const int &y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {
}

Coord::Coord(const std::vector<float> &xy) {
    // assert(xy.size() == 2);
    x = xy.at(0);
    y = xy.at(1);
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
    return Angle(atan2f(y, x));
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

Coord operator/(const Coord &other, float scalar) {
    return {other.x / scalar, other.y / scalar};
}

Coord &Coord::add(const Angle &dir, const float &dist) {
    x += dist * cosf(dir.rad);
    y += dist * sinf(dir.rad);
    return *this;
}

DirectedCoord Coord::lookAt(const Coord &other) {
    float _x = this->x;
    float _y = this->y;
    float dx = other.x - _x;
    float dy = other.y - _y;
    float _angle = atan2f(dy, dx);
    DirectedCoord result(_x, _y, _angle);
    return result;
}

DirectedCoord DirectedCoord::walk(const DirectedCoord &delta) const {
    // The same as toRCS() but fixed
    // Didn't touch the other one because of backwards compatibility fear
    float x1 = this->coord.x;
    float y1 = this->coord.y;
    float a1 = this->angle.rad;
    float x2 = delta.coord.x;
    float y2 = delta.coord.y;
    float a2 = delta.angle.rad;
    DirectedCoord out(0, 0, 0);
    // Transformation Matrix
    out.angle = Angle().normalize(a1 + a2);
    out.coord.x = x1 + cosf(out.angle.rad) * x2 - sinf(out.angle.rad) * y2;
    out.coord.y = y1 + sinf(out.angle.rad) * x2 + cosf(out.angle.rad) * y2;
    return out;
}

DirectedCoord DirectedCoord::toRCS(const DirectedCoord &origin) const {
    DirectedCoord out(this->coord.x, this->coord.y, this->angle.rad);
    // transform from wcs to rcs
    // 1) POSE
    // 1a) add translation
    out.coord.x -= origin.coord.x;
    out.coord.y -= origin.coord.y;
    // 1b) rotate local coordinates according to alpha!
    float alpha = -origin.angle.rad;
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);
    float _x = out.coord.x * cosalpha - out.coord.y * sinalpha;
    out.coord.y = out.coord.x * sinalpha + out.coord.y * cosalpha;
    out.coord.x = _x;
    return out;
}

DirectedCoord DirectedCoord::toWCS(const DirectedCoord &origin) const {
    DirectedCoord out(this->coord.x, this->coord.y, this->angle.rad);
    // 1) POSE
    // 1a) de-rotate local coordinates according to alpha!
    float alpha = origin.angle.rad;
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);
    float _x = out.coord.x * cosalpha - out.coord.y * sinalpha;
    out.coord.y = out.coord.x * sinalpha + out.coord.y * cosalpha;
    out.coord.x = _x;
    // 1b) add translation
    out.coord.x += origin.coord.x;
    out.coord.y += origin.coord.y;

    return out;
}

Angle::Angle() : deg(0), rad(0.0f) {
}

Angle::Angle(const float &rad) {
    set(rad);
}

Angle::Angle(const int &deg) {
    set(deg);
}

float Angle::normalize(const float &rad) {
    return remainder(rad, 2.f * M_PI_F);
}

double Angle::normalize(const double &rad) {
    return remainder(rad, 2.f * M_PI);
}

int Angle::normalize(const int &deg) {
    return remainder(deg, 360);
}

float Angle::pos_normalize(const float &rad) {
    const float &out = Angle::normalize(rad);
    return (out < 0.f) ? out + 2.f * M_PI_F : out;
}

double Angle::pos_normalize(const double &rad) {
    const double &out = Angle::normalize(rad);
    return (out < 0.0) ? out + 2.0 * M_PI : out;
}

int Angle::pos_normalize(const int &deg) {
    const int &out = Angle::normalize(deg);
    return (out < 0) ? out + 360 : out;
}

Angle Angle::operator-(const Angle &other) const {
    return {this->rad - other.rad};
}

Angle Angle::operator+(const Angle &other) const {
    return {this->rad + other.rad};
}

Angle &Angle::set(const Angle &obj) {
    this->rad = obj.rad;
    this->deg = obj.deg;
    return *this;
}

Angle &Angle::set(const float &rad) {
    this->rad = Angle::normalize(rad);
    this->deg = static_cast<int>(roundf(RAD_TO_DEG * rad));
    return *this;
}

Angle &Angle::set(const int &deg) {
    this->deg = Angle::normalize(deg);
    this->rad = static_cast<float>(DEG_TO_RAD * deg);
    return *this;
}

Angle &Angle::add(const Angle &other) {
    this->set(other.rad + this->rad);
    return *this;
}

Angle &Angle::sub(const Angle &other) {
    this->set(other.rad - this->rad);
    return *this;
}

Angle Angle::dist(const Angle &ang) const {
    return Angle(*this - ang);
}

DirectedCoord::DirectedCoord() : angle(Angle()), coord(Coord()) {
}

DirectedCoord::DirectedCoord(const float &x, const float &y, const float &rad) : angle({rad}), coord(Coord(x, y)) {
}

DirectedCoord::DirectedCoord(const Angle &a, const Coord &c) : angle(a), coord(c) {
}

DirectedCoord::DirectedCoord(const Coord &c, const Angle &a) : angle(a), coord(c) {
}

DirectedCoord::DirectedCoord(const DirectedCoord &other) : angle(other.angle), coord(other.coord) {
}

DirectedCoord::DirectedCoord(const std::vector<float> &vals) : angle({vals.at(2)}), coord(vals.at(0), vals.at(1)) {
}

DirectedCoord &DirectedCoord::add(const DirectedCoord &other) {
    this->angle.add(other.angle);
    this->coord.add(other.coord);
    return *this;
}

DirectedCoord &DirectedCoord::sub(const DirectedCoord &other) {
    this->angle.sub(other.angle);
    this->coord.sub(other.coord);
    return *this;
}

DirectedCoord DirectedCoord::operator-(const DirectedCoord &other) const {
    return {this->angle - other.angle, this->coord - other.coord};
}

DirectedCoord DirectedCoord::operator+(const DirectedCoord &other) const {
    return {this->angle + other.angle, this->coord + other.coord};
}

std::ostream &operator<<(std::ostream &s, const Angle &obj) {
    return (s << obj.rad << "(rad)");
}

std::ostream &operator<<(std::ostream &s, const Coord &obj) {
    return (s << obj.x << "," << obj.y);
}

std::ostream &operator<<(std::ostream &s, const DirectedCoord &obj) {
    return (s << obj.coord << "@" << obj.angle);
}
} // namespace MathToolbox
