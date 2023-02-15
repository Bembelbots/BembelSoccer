#pragma once

#include "angle.h"
#include "../serialize/serializer.h"
#include <iosfwd>

class DirectedCoord;
class Coord {
public:
    Coord();
    Coord(const float &x, const float &y);
    Coord(const int &x, const int &y);
    /* Coord(const std::vector<float> &xy); */

    explicit Coord(const Angle &); // create unit vector for this angle

    float norm2() const;
    // parse from string, should match operator<<
    //static Coord fromString(const std::string& s);

    Coord normalized() const;

    Coord clamp(float maxVal) const;
    Coord scale_maxValue_to (float maxVal) const;

    Angle direction() const;

    // get distance from 'target' to this coord
    float dist(const Coord &target) const;

    // get distance from Coord(0,0) (coords origin) to this coord
    float dist() const;

    // get angle for this coordinate (from Coord(0,0))
    Angle angle() const;

    float dot(const Coord &other) const;

    // get angle from 'target' arbitrary coordinate to this Coord TODO!!!!
    // Angle angle(const Coord& target) const;

    // add 'other' to this coordinate
    Coord &add(const Coord &other);
    // substract 'other' from this coordinate
    Coord &sub(const Coord &other);

    Coord rotate(const Angle &) const;

    Coord operator-(const Coord &other) const;
    Coord operator+(const Coord &other) const;
    Coord operator*(const Coord &other) const;

    inline Coord &operator-=(const Coord &other) { return *this = *this - other; }

    inline Coord &operator+=(const Coord &other) { return *this = *this + other; }

    friend Coord operator*(float scalar, const Coord &other);
    friend Coord operator*(const Coord &other, float scalar);
    friend Coord operator/(float scalar, const Coord &other);
    friend Coord operator/(const Coord &other, float scalar);

    bool operator==(const Coord &other) const;
    bool operator!=(const Coord &other) const;

    Coord &operator/=(float scalar);

    // add 'dist' towards 'dir' to coordinate
    Coord &add(const Angle &dir, const float &dist);

    // get distance from coord to line with startpoint start and endpoint end
    Coord closestPointOnLine(Coord start, Coord end, bool onLine = false) const;

    DirectedCoord lookAt(const Coord &other);

    float x;
    float y;
};

std::ostream &operator<<(std::ostream &s, const Coord &obj);

SERIALIZE(Coord, {
    return {key, str(value.x, 4) + "," + str(value.y, 4)};
});
