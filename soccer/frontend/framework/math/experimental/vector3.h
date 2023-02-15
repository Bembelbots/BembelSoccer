#include <cmath>

namespace MathToolbox {
template<typename T>
class Vector3;

typedef Vector3<int> Vector3i;
typedef Vector3<float> Vector3f;

template<typename T>

class Vector3 {
public:
    T x, y, z;

    Vector3() : x(0), y(0), z(0){};

    Vector3(T x1, T x2, T x3) : x(x1), y(x2), z(x3){};

    // Length  =  distance to origin
    float dist() { return sqrt(x * x + y * y + z * z); }

    // Unit vector
    Vector3<float> unit() {
        Vector3<float> unit_vec;

        if (x == 0 && y == 0 && z == 0) {
            return unit_vec;
        }

        unit_vec.x = x / dist();
        unit_vec.y = y / dist();
        unit_vec.z = z / dist();

        return unit_vec;
    }

    // Addition
    Vector3<T> operator+=(const Vector3<T> &v) {
        x += v.x;
        y += v.y;
        z += v.z;

        return *this;
    }

    // Addition
    Vector3<T> operator+(const Vector3<T> &v) { return Vector3<T>(*this) += v; }

    // Substraction
    Vector3<T> operator-=(const Vector3<T> &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;

        return *this;
    }

    // Substraction
    Vector3<T> operator-(const Vector3<T> &v) { return Vector3<T>(*this) -= v; }

    // Scalar product
    T operator*=(const Vector3<T> &v) { return x * v.x + y * v.y + z * v.z; }

    // Scalar product
    T operator*(const Vector3<T> &v) { return Vector3<T>(*this) *= v; }

    // Scalar multiplication
    Vector3<T> operator*=(const T &scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;

        return *this;
    }

    // Scalar multiplication
    Vector3<T> operator*(const T &scalar) { return Vector3<T>(*this) *= scalar; }

    // Division by a scalar
    Vector3<T> operator/=(const T &scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;

        return *this;
    }

    // Division by a scalar
    Vector3<T> operator/(const T &scalar) { return Vector3<T>(*this) /= scalar; }

    // Vector/Cross Product
    Vector3<T> CrossProduct(const Vector3<T> &v) {
        x = y * v.z - z * v.y;
        y = z * v.x - x * v.z;
        z = x * v.y - y * v.x;

        return *this;
    }
};

template<typename T>

std::ostream &operator<<(std::ostream &os, const Vector3<T> &v) {
    os << v.x << " " << v.y << " " << v.z;
    return os;
}
} // namespace MathToolbox
