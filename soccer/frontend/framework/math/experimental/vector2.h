#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include "angle.h"

namespace MathToolbox {

template<typename T>
class Vector2;

typedef Vector2<int> Vector2i;
typedef Vector2<float> Vector2f;

template<typename T>
class Vector2 {
public:
    T x, y;

    Vector2() : x(0), y(0){};

    Vector2(T x1, T y1) : x(x1), y(y1){};

    explicit Vector2(const std::vector<T> &xy) : x(xy.at(0)), y(xy.at(1)){};

    template<typename X>
    explicit Vector2(Vector2<X> v) {
        x = static_cast<T>(v.x);
        y = static_cast<T>(v.y);
    }

    float len() { return hypotf(x, y); }

    Vector2<float> unit() {
        Vector2<float> unit_vec(*this);

        if (*this == NullVector()) {
            return unit_vec;
        }

        return unit_vec /= len();
    }

    Angle angle() { return Angle::fromRadians(atan2f(y, x)); }

    T det(const Vector2<T> &v) const { return x * v.y - y * v.x; }

    // see: https://stackoverflow.com/a/16544330
    /// Returns the rotational difference, the angle between two vectors which can be negative.
    /// i.e. rotDiff( (1,0), (0,1)) will return 90 degrees, and rotDiff( (0,1), (1,0) ) will return -90 degrees
    /// \param v
    /// \return
    Angle rotationDiff(const Vector2<T> &v) {
        Vector2<T> u = unit();
        Vector2<T> vcopy = Vector2<T>(v);
        Vector2<T> v_u = vcopy.unit();
        float _dot = u * v_u;
        float _det = u.det(v_u);
        return Angle::fromRadians(atan2f(_det, _dot));
    }

    /// Returns the angle between two vectors. This will always be positive.
    /// i.e. i.e. angleBetween( (1,0), (0,1)) will return 90 degrees
    /// \param v
    /// \return
    Angle angleBetween(const Vector2<T> &v) { return Angle::fromRadians(std::abs(float(rotationDiff(v)))); }

    Vector2<float> rotateClockwise(const Angle &angle) const {
        float sincache = sinf(angle);
        float coscache = cosf(angle);
        float new_x = x * coscache - y * sincache;
        float new_y = x * sincache + y * coscache;
        return Vector2<float>(new_x, new_y);
    }

    static Vector2<T> NullVector() { return Vector2<T>(); };
};

template<typename T>
bool operator==(const Vector2<T> &v, const Vector2<T> &other) {
    return v.x == other.x && v.y == other.y;
}

template<typename T>
bool operator!=(const Vector2<T> &v, const Vector2<T> &other) {
    return v.x != other.x || v.y != other.y;
}

template<typename T>
T operator*(const Vector2<T> &v, const Vector2<T> &other) {
    return v.x * other.x + v.y * other.y;
}

template<typename T>
Vector2<T> operator+=(Vector2<T> &v, const Vector2<T> &other) {
    v.x += other.x;
    v.y += other.y;
    return v;
}

template<typename T>
Vector2<T> operator+(const Vector2<T> &v, const Vector2<T> &other) {
    Vector2<T> tmp(v);
    return tmp += other;
}

template<typename T>
Vector2<T> operator-=(Vector2<T> &v, const Vector2<T> &other) {
    v.x -= other.x;
    v.y -= other.y;
    return v;
}

template<typename T>
Vector2<T> operator-(const Vector2<T> &v, const Vector2<T> &other) {
    Vector2<T> tmp(v);
    return tmp -= other;
}

template<typename T>
Vector2<T> operator*=(Vector2<T> &v, const T &scalar) {
    v.x *= scalar;
    v.y *= scalar;
    return v;
}

template<typename T>
Vector2<T> operator*(const Vector2<T> &v, const T &scalar) {
    Vector2<T> tmp(v);
    return tmp *= scalar;
}

template<typename T>
Vector2<T> operator*=(const T &scalar, const Vector2<T> &v) {
    Vector2<T> tmp(v);
    return tmp *= scalar;
}

template<typename T>
Vector2<T> operator*(const T &scalar, const Vector2<T> &v) {
    Vector2<T> tmp(v);
    return tmp *= scalar;
}

template<typename T>
Vector2<T> operator/=(Vector2<T> &v, const T &scalar) {
    v.x /= scalar;
    v.y /= scalar;
    return v;
}

template<typename T>
Vector2<T> operator/(const Vector2<T> &v, const T &scalar) {
    Vector2<T> tmp(v);
    return tmp /= scalar;
}

template<typename T>
Vector2<T> operator/=(const T &scalar, const Vector2<T> &v) {
    Vector2<T> tmp(v);
    return tmp /= scalar;
}

template<typename T>
Vector2<T> operator/(const T &scalar, const Vector2<T> &v) {
    Vector2<T> tmp(v);
    return tmp /= scalar;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const Vector2<T> &v) {
    os << v.x << " " << v.y;
    return os;
}

} // namespace MathToolbox
