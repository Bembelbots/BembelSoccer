#pragma once
#include "vector2.h"
#include "linesegment.h"

namespace MathToolbox {

template<typename T>
class Circle {
public:
    Circle(Vector2<T> midpoint, T radius) {
        this->midpoint = midpoint;
        this->radius = radius;
    }

    Circle() {
        this->midpoint = Vector2<T>(0, 0);
        this->radius = 0;
    }

    bool containsPoint(Vector2<T> point) { return ((midpoint - point).len() < radius); }

    T distanceOfPointToCircumference(Vector2<T> point) { return std::abs((midpoint - point).len() - radius); }

    Vector2<T> midpoint;
    T radius;
};
} // namespace MathToolbox
