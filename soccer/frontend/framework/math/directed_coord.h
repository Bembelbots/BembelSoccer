#pragma once

#include "angle.h"
#include "coord.h"
#include "../serialize/serializer.h"

#include <iosfwd>

class DirectedCoord {
public:
    DirectedCoord();

    DirectedCoord(const DirectedCoord &other);

    // parse from string, should match operator<<
    //static DirectedCoord fromString(const std::string& s);

    // TODO / FIXME: STOP USING vector<float> as coord / coord+angle!!!!
    // as long as it still exists in the framework, we need this:
    /* DirectedCoord(const std::vector<float> &vals); */

    DirectedCoord(const Angle &a, const Coord &c);
    DirectedCoord(const Coord &c, const Angle &a); // hrhr take them' all ;D

    DirectedCoord(const float &x, const float &y, const Angle &rad);

    // Construct with null angle
    explicit DirectedCoord(const Coord &c);
    DirectedCoord(float x, float y);

    // DirectedCoord(const float& x, const float& y, const int& deg);
    // DirectedCoord(const int& x, const int& y, const float& rad);
    // DirectedCoord(const int& x, const int& y, const int& deg);

    float x() const { return coord.x; }
    float y() const { return coord.y; }
    float rad() const { return angle.rad(); }

    DirectedCoord &add(const DirectedCoord &other);
    DirectedCoord &sub(const DirectedCoord &other);

    // TODO: need this badly, this is already somewhere
    // Coord get_intersection(const Angle& o_angle);

    //DirectedCoord normalized() const;
    DirectedCoord clamp(float maxVal) const;

    // walkTo goes starts from one coordinate and walks a relative path
    // x.walk(y) starts at x with direction x.angle
    // and walks by y and turns by angle y.angle
    // (same as toRCS but with correct angles)
    DirectedCoord walk(const DirectedCoord &delta) const;

    inline DirectedCoord rotate(const Angle &theta) const { return DirectedCoord{coord, angle + theta}; }

    // Linear interpolation between this coord and other.
    // Parameter delta should be in [0, 1], with
    // this.interpolate(other, 0) == this and
    // this.interpolate(other, 1) == other.
    DirectedCoord interpolate(const DirectedCoord &other, float delta) const;

    // transform to RCS/WCS (UNTESTED=?=)
    DirectedCoord toRCS(const DirectedCoord &origin) const;
    DirectedCoord toWCS(const DirectedCoord &origin) const;

    DirectedCoord operator=(const DirectedCoord &other);
    DirectedCoord &operator+=(const DirectedCoord &other);

    DirectedCoord operator-(const DirectedCoord &other) const;
    DirectedCoord operator+(const DirectedCoord &other) const;
    DirectedCoord operator*(const DirectedCoord &other) const;

    bool isNull() const;

    Angle angle;
    Coord coord;
};

inline DirectedCoord operator*(const DirectedCoord &c, float s) {
    return DirectedCoord{c.coord * s, c.angle * s};
}

inline DirectedCoord operator*(float s, const DirectedCoord &c) {
    return c * s;
}

std::ostream &operator<<(std::ostream &s, const DirectedCoord &obj);

SERIALIZE(DirectedCoord, {
    return {key, str(value.coord.x, 4) + "," + str(value.coord.y, 4) + "@" + str(value.angle.rad(), 4)};
});