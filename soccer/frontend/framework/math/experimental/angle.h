#pragma once
#include <iostream>

#include "constants.h"
#include "radians.h"
#include "degrees.h"

namespace MathToolbox {

class Angle {
public:
    Angle() = default;

    explicit Angle(float radians)
        : deg(Degrees(radians))
        , rad(Radians(radians)) {}

    // cppcheck-suppress noExplicitConstructor
    Angle(Degrees angle)
        : deg(angle)
        , rad(Radians(deg)) {}

    // cppcheck-suppress noExplicitConstructor
    Angle(Radians angle)
        : deg(Degrees(angle))
        , rad(angle) {}

    static Angle fromDegrees(float angle) { return Degrees(angle); }

    static Angle fromRadians(float angle) { return Radians(angle); }

    float getAsDegrees() const { return float(deg); }

    float getAsRadians() const { return float(rad); }

    Angle set(float angle) {
        rad = Radians(angle);
        deg = Degrees(rad);
        return *this;
    }

    operator float() const { return rad; }

    operator Radians() const { return rad; }

    operator Degrees() const { return deg; }

    friend bool operator>(const Angle &angle, const Degrees &other);

    friend bool operator>=(const Angle &angle, const Degrees &other);

    friend bool operator<(const Angle &angle, const Degrees &other);

    friend bool operator<=(const Angle &angle, const Degrees &other);

    friend bool operator>(const Degrees &other, const Angle &angle);

    friend bool operator>=(const Degrees &other, const Angle &angle);

    friend bool operator<(const Degrees &other, const Angle &angle);

    friend bool operator<=(const Degrees &other, const Angle &angle);

    friend bool operator>(const Angle &angle, const Radians &other);

    friend bool operator>=(const Angle &angle, const Radians &other);

    friend bool operator<(const Angle &angle, const Radians &other);

    friend bool operator<=(const Angle &angle, const Radians &other);

    friend bool operator>(const Radians &other, const Angle &angle);

    friend bool operator>=(const Radians &other, const Angle &angle);

    friend bool operator<(const Radians &other, const Angle &angle);

    friend bool operator<=(const Radians &other, const Angle &angle);

    friend bool operator>(const Angle &other, const Angle &angle);

    friend bool operator>=(const Angle &other, const Angle &angle);

    friend bool operator<(const Angle &other, const Angle &angle);

    friend bool operator<=(const Angle &other, const Angle &angle);

    bool operator==(const float &other) = delete;

    friend std::ostream &operator<<(std::ostream &os, const Angle &angle);

private:
    Degrees deg;
    Radians rad;
};

template<typename T>
Angle operator+=(Angle &angle, const T &other) {
    return angle = Angle((float)angle + (float)other);
}

template<typename T>
float operator+(const Angle &angle, const T &other) {
    return (float)angle + (float)other;
}

template<typename T>
Angle operator-=(Angle &angle, const T &other) {
    return angle = Angle((float)angle - (float)other);
}

template<typename T>
float operator-(const Angle &angle, const T &other) {
    return (float)angle - (float)other;
}

template<typename T>
float operator/=(Angle &angle, const T &other) {
    return angle = Angle((float)angle / (float)other);
}

template<typename T>
float operator/(const Angle &angle, const T &other) {
    return (float)angle / (float)other;
}

template<typename T>
float operator*=(Angle &angle, const T &other) {
    return angle = Angle((float)angle * (float)other);
}

template<typename T>
float operator*(const Angle &angle, const T &other) {
    return (float)angle * (float)other;
}

// TODO: allow operators with Degrees by converting them to Radians
float operator+=(const Angle &angle, const Degrees &other) = delete;

float operator+(const Angle &angle, const Degrees &other) = delete;

float operator-=(const Angle &angle, const Degrees &other) = delete;

float operator-(const Angle &angle, const Degrees &other) = delete;

float operator/=(const Angle &angle, const Degrees &other) = delete;

float operator/(const Angle &angle, const Degrees &other) = delete;

float operator*=(const Angle &angle, const Degrees &other) = delete;

float operator*(const Angle &angle, const Degrees &other) = delete;
} // namespace MathToolbox
