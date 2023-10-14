#pragma once
#include "vector2.h"

namespace MathToolbox {

enum CoordTypes { RCS, WCS, MCS, ICS, CCS };

std::ostream &operator<<(std::ostream &os, const CoordTypes &coord_type);

template<CoordTypes T>
class DirectedCoord;

template<CoordTypes T>
class Coord : public Vector2<float> {
public:
    Coord() = default;

    Coord(float x, float y) : Vector2<float>(x, y){};

    Coord(const Coord<T> &other) : Vector2<float>(other.x, other.y){};

    explicit Coord(const std::vector<float> &xy) : Vector2<float>(xy){};

    explicit Coord(const Vector2<float> &xy) : Vector2<float>(xy){};

    operator DirectedCoord<T>() { return DirectedCoord<T>(*this, this->angle()); }
    
    Coord<T> &operator=(const Coord<T> &other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    Coord<T> rotateClockwise(const Angle &angle) const { return (Coord<T>)Vector2<float>::rotateClockwise(angle); }

    DirectedCoord<T> lookAt(const Coord<T> &other) { return DirectedCoord<T>(*this, (other - *this).angle()); }
};

template<CoordTypes T, CoordTypes X>
Coord<T> operator+=(Coord<T> &c, const Coord<X> &other) = delete;

template<CoordTypes T, CoordTypes X>
Coord<T> operator+(const Coord<T> &c, const Coord<X> &other) = delete;

template<CoordTypes T, CoordTypes X>
Coord<T> operator-=(Coord<T> &c, const Coord<X> &other) = delete;

template<CoordTypes T, CoordTypes X>
Coord<T> operator-(const Coord<T> &c, const Coord<X> &other) = delete;

template<CoordTypes T, CoordTypes X>
float operator*=(Coord<T> &c, const Coord<X> &other) = delete;

template<CoordTypes T, CoordTypes X>
float operator*(const Coord<T> &c, const Coord<X> &other) = delete;

template<CoordTypes T>
Coord<T> operator+=(Coord<T> &c, const Coord<T> &other) {
    return c = Coord<T>(c += (Vector2<float>)other);
}

template<CoordTypes T>
Coord<T> operator+(const Coord<T> &c, const Coord<T> &other) {
    return Coord<T>(c + (Vector2<float>)other);
}

template<CoordTypes T>
Coord<T> operator-=(Coord<T> &c, const Coord<T> &other) {
    return c = Coord<T>(c -= (Vector2<float>)other);
}

template<CoordTypes T>
Coord<T> operator-(const Coord<T> &c, const Coord<T> &other) {
    return Coord<T>(c - (Vector2<float>)other);
}

template<CoordTypes T>
float operator*(const Coord<T> &c, const Coord<T> &other) {
    return c * (Vector2<float>)other;
}

template<CoordTypes T>
class DirectedCoordBase {
public:
    Angle angle;
    Coord<T> coord;

    DirectedCoordBase() : angle(), coord(){};

    explicit DirectedCoordBase(const Coord<T> &c) : angle(), coord(c){};

    DirectedCoordBase(const Coord<T> &c, const Angle &a) : angle(a), coord(c){};

    template<CoordTypes X>
    explicit DirectedCoordBase(const DirectedCoordBase<X> &other) : angle(other.angle), coord(other.coord) {}
};

template<CoordTypes T>
class DirectedCoord : public DirectedCoordBase<T> {
    using DirectedCoordBase<T>::DirectedCoordBase;

    // cppcheck-suppress unusedFunctions
    DirectedCoord<T> walk(const DirectedCoord<T> &delta) const {
        DirectedCoord<T> out(*this);
        out.angle += delta.angle;
        out.coord += delta.coord.rotateClockwise(out.angle);
        return out;
    }
};

template<>
class DirectedCoord<WCS> : public DirectedCoordBase<WCS> {
public:
    using DirectedCoordBase<WCS>::DirectedCoordBase;

    DirectedCoord<RCS> toRCS(DirectedCoord<WCS> mypos) {
        DirectedCoord<RCS> out(*this);
        out.coord -= (Vector2<float>)mypos.coord;
        out.angle -= mypos.angle;
        out.coord = out.coord.rotateClockwise(out.angle);
        return out;
    }
};

template<CoordTypes T>
std::ostream &operator<<(std::ostream &os, const Coord<T> &c) {
    os << T << " < " << c.x << ", " << c.y << " >";
    return os;
}

template<CoordTypes T>
std::ostream &operator<<(std::ostream &os, const DirectedCoord<T> &dc) {
    os << "Directed " << T << " < [ " << dc.coord.x << ", " << dc.coord.y << " ]"
       << ", " << dc.angle << " >";
    return os;
}
} // namespace MathToolbox
