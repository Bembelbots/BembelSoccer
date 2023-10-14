#pragma once

#include "constants.h"
#include "../serialize/serializer.h"

#include <iosfwd>

struct Rad {
    float v;

    Rad operator-() const { return Rad{-v}; }
};

struct Deg {
    float v;

    Deg operator-() const { return Deg{-v}; }
};

#ifndef DISABLE_ANGLE_OPERATORS
constexpr inline Rad operator""_rad(long double f) {
    return Rad{static_cast<float>(f)};
}

constexpr inline Deg operator""_deg(long double f) {
    return Deg{static_cast<float>(f)};
}

constexpr inline Deg operator""_deg(unsigned long long int f) {
    return Deg{static_cast<float>(f)};
}
#endif

/** Angle class (high-lvl angle handling)
 *  - automated normalization and constraining
 *  - change 'angle' through methods:
 *    - DO _NOT_ WRITE ::rad and ::deg directly
 *    - READING from ::rad and ::deg is encouraged and prefered
 **/
class Angle {
public:
    Angle() : _rad(0) {}
    // Allow implicit conversion from Rad / Deg for less verbose code
    Angle(Deg deg) : Angle(Rad{deg.v * DEG_TO_RAD}) {}  // cppcheck-suppress noExplicitConstructor
    Angle(Rad rad) : _rad(normalize(rad.v)) {}          // cppcheck-suppress noExplicitConstructor

    // parse from string, should match operator<<
    //static Angle fromString(const std::string& s);

    // normalization stuff, static to be used everywhere EXCLUSIVLY!
    // radians --> [-pi ... pi]
    static float normalize(const float &rad);
    static bool isNormalized(float rad);

    // use these to change the underlying angle
    Angle &set(const Angle &obj);

    Angle &add(const Angle &other);
    Angle &sub(const Angle &other);

    Angle clamp(float maxVal) const;

    // returns new Angle-object with shortest(!) turn dist between this and 'ang'
    // for [-180 ... 180] and [-pi ... pi] constrained angles,
    // this is always just the _normalized_ difference ;)
    Angle dist(const Angle &ang) const;

    // (weighted) merge two angles ... this is not as simple as one might think
    // nevertheless, it's never used by anyone inside the framework, thus
    // mark it here as potential TODO, but don't implement it...
    Angle merge(const Angle &other, const float &my_weight = 1.f, const float &other_weight = 1.f);

    // Moves radians into rage [0 ... 2*pi]
    float posNormalized() const;

    inline float deg() const { return _rad * RAD_TO_DEG; }
    inline float rad() const { return _rad; }

    Angle &operator+=(const Angle &other);
    Angle &operator-=(const Angle &other);
    Angle &operator*=(float scale);
    Angle operator-(const Angle &other) const;
    Angle operator+(const Angle &other) const;
    Angle operator*(const Angle &other) const;
    Angle operator*(float scale) const;
    Angle operator/(float scale) const;
    Angle operator-() const;
    bool operator<(const Angle &other) const;
    bool operator>(const Angle &other) const;
    bool operator==(const Angle &other) const;

private:
    // UNDERLYING DATA ITEM
    // [-pi .. pi] (counter-clock wise)
    float _rad;
};

std::ostream &operator<<(std::ostream &s, const Angle &);

SERIALIZE(Angle, {
    return {key, str(value.deg(), 4)};
});
