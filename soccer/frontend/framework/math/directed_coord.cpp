#include "directed_coord.h"

#include <cmath>
#include <ostream>

DirectedCoord DirectedCoord::clamp(float maxVal) const {
    return DirectedCoord(coord.clamp(maxVal), angle.clamp(maxVal));
}

DirectedCoord Coord::lookAt(const Coord &other) {
    float _x = this->x;
    float _y = this->y;
    float dx = other.x - _x;
    float dy = other.y - _y;
    float _angle = atan2f(dy, dx);
    DirectedCoord result(_x, _y, Rad{_angle});
    return result;
}

DirectedCoord DirectedCoord::walk(const DirectedCoord &delta) const {
    // The same as toRCS() but fixed
    // Didn't touch the other one because of backwards compatibility fear
    float x1 = this->coord.x;
    float y1 = this->coord.y;
    float a1 = this->angle.rad();
    float x2 = delta.coord.x;
    float y2 = delta.coord.y;
    float a2 = delta.angle.rad();
    DirectedCoord out(0, 0, Rad{0});
    // Transformation Matrix
    //out.angle = Angle().normalize( a1 + a2);
    out.angle.set(Rad{a1 + a2});
    out.coord.x = x1 + cosf(out.angle.rad()) * x2 - sinf(out.angle.rad()) * y2;
    out.coord.y = y1 + sinf(out.angle.rad()) * x2 + cosf(out.angle.rad()) * y2;
    return out;
}

DirectedCoord DirectedCoord::interpolate(const DirectedCoord &other, float delta) const {
    return *this + delta * (other - *this);
}

DirectedCoord DirectedCoord::toRCS(const DirectedCoord &origin) const {
    DirectedCoord out(*this);
    // transform from wcs to rcs
    // 1) POSE
    // 1a) add translation
    out.coord.x -= origin.coord.x;
    out.coord.y -= origin.coord.y;
    // 1b) rotate local coordinates according to alpha!
    float alpha = -origin.angle.rad();
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);
    float _x = out.coord.x * cosalpha - out.coord.y * sinalpha;
    out.coord.y = out.coord.x * sinalpha + out.coord.y * cosalpha;
    out.coord.x = _x;
    return out;
}

DirectedCoord DirectedCoord::toWCS(const DirectedCoord &origin) const {
    DirectedCoord out(*this);
    // 1) POSE
    // 1a) de-rotate local coordinates according to alpha!
    float alpha = origin.angle.rad();
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

DirectedCoord::DirectedCoord() : angle(Angle()), coord(Coord()) {
}

DirectedCoord::DirectedCoord(const float &x, const float &y, const Angle &rad) : angle(rad), coord(Coord(x, y)) {
}

DirectedCoord::DirectedCoord(const Angle &a, const Coord &c) : angle(a), coord(c) {
}

DirectedCoord::DirectedCoord(const Coord &c, const Angle &a) : angle(a), coord(c) {
}

DirectedCoord::DirectedCoord(const DirectedCoord &other) : angle(other.angle), coord(other.coord) {
}

DirectedCoord::DirectedCoord(const Coord &c) : angle(), coord(c) {
}
DirectedCoord::DirectedCoord(float x, float y) : DirectedCoord(Coord{x, y}) {
}

/* DirectedCoord::DirectedCoord(const std::vector<float> &vals) */
/*     : angle( { */
/*     vals.at(2) */
/* }), coord(vals.at(0), vals.at(1)) {} */

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

DirectedCoord DirectedCoord::operator=(const DirectedCoord &other) {
    angle = other.angle;
    coord = other.coord;
    return *this;
}

DirectedCoord DirectedCoord::operator-(const DirectedCoord &other) const {
    return {this->angle - other.angle, this->coord - other.coord};
}

DirectedCoord DirectedCoord::operator+(const DirectedCoord &other) const {
    return {this->angle + other.angle, this->coord + other.coord};
}

DirectedCoord DirectedCoord::operator*(const DirectedCoord &other) const {
    return {this->angle * other.angle, this->coord * other.coord};
}

DirectedCoord &DirectedCoord::operator+=(const DirectedCoord &other) {
    this->coord = this->coord + other.coord;
    this->angle = this->angle + other.angle;
    return *this;
}

bool DirectedCoord::operator==(const DirectedCoord &other) const {
    return coord == other.coord && angle == other.angle;
}

bool DirectedCoord::isNull() const {
    return coord.x == 0 && coord.y == 0 && angle.rad() == 0;
}

std::ostream &operator<<(std::ostream &s, const DirectedCoord &obj) {
    return (s << obj.coord << "@" << obj.angle);
}
