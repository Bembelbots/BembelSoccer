#pragma once

#include "vector2.h"
#include "line.h"

namespace MathToolbox {

template<typename T>
class LineSegment {
public:
    LineSegment(Vector2<T> first, Vector2<T> second) {
        this->first = first;
        this->second = second;
    }

    LineSegment() {
        this->first = Vector2<T>{0, 0};
        this->second = Vector2<T>{0, 0};
    }

    Line<T> getPerpendicularLine() {
        Vector2<T> dir = getDirection();
        return Line<T>::fromDirectionAndPoint(Vector2<T>{-dir.y, dir.x}, getMidpoint());
    }

    double distanceToPoint(const Vector2<T> &v) { return fabs(getDirection().det(v)); }

    double distanceToLineSegment(const LineSegment<T> &lineSegment) {
        return distanceToPoint(lineSegment.getMidpoint());
    }

    Vector2<T> getMidpoint() const { return (first + second) / 2.0f; }

    Vector2<T> getDirection() const { return (first - second).unit(); }

    LineSegment merge(const LineSegment &toMergeWith) {
        return LineSegment{(first + toMergeWith.first) / 2.0f, (second + toMergeWith.second) / 2.0f};
    }

    Vector2<T> first;
    Vector2<T> second;
};
} // namespace MathToolbox