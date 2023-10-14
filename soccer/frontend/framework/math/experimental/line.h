#pragma once

#include <boost/optional.hpp>
#include "vector2.h"

namespace MathToolbox {

template<typename T>
class Line {
public:
    // Generative form of line (base + t * direction)
    Line(Vector2<T> p1, Vector2<T> p2) {
        direction = (p2 - p1).unit();
        this->base = p1;
    }
    Line(Coord p1, Coord p2){
        this = Line({p1.x, p1.y},{p2.x,p2.y});

    }

    static Line fromDirectionAndPoint(Vector2<T> direction, Vector2<T> base) {
        Line l = Line();
        l.direction = direction;
        l.base = base;
        return l;
    }

    /*
         * TODO: resolve circular dependency
        Line(LineSegment<T> l) {
            //Line(l.first, l.second);
        } */

    /*
         * https://stackoverflow.com/a/565282
         *
         *
         **/
    ///
    /// \param line2
    /// \param minAngle minimum angle between the two lines, in degrees
    /// \return the point at which the lines intersect, or nullptr, if the angle between the lines is below minangle.
    boost::optional<Vector2f> intersection(Line<T> line2, double minAngle) {
        if ((direction.angleBetween(line2.direction)).getAsDegrees() < minAngle) {
            return boost::none;
        }
        float t = (base - line2.base).det(direction) / line2.direction.det(direction);
        return base + t * direction;
    }

    Vector2<T> direction;
    Vector2<T> base;

private:
    Line(){};
};
} // namespace MathToolbox