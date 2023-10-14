#include <framework/math/coord.h>
using line_t = std::pair<Coord, Coord>;

namespace bbvision {

//@TODO why am I using dy / dx ? I should be using vector math.

// TODO Implement in mathtoolbox.cpp and delete this file.
// Use vector math instead of function math
// Function math is even more unreliable!!!!
// TODO: get Jonathan to do it

struct StraightLineEq {
    float gradient{0};
    float c{0};

    StraightLineEq() = default;
    StraightLineEq(const StraightLineEq &) = default;
    constexpr StraightLineEq(float _m, float _c) : gradient(_m), c(_c) {}

    bool operator==(const StraightLineEq &o) const {
        return gradient == o.gradient && c == o.c;
    }

    float intersectionOffset(const StraightLineEq &o) const {
        float gradient_perpendicular = -1/gradient;
        return std::abs((gradient * o.gradient) - gradient_perpendicular);
    }

    Coord intersect(const StraightLineEq &o) const {
        Coord intersect;

        intersect.x = (o.c - c) / (gradient - o.gradient);
        intersect.y = gradient * intersect.x + c;

        return intersect;
    }

    Coord getCoord(float x) {
        return Coord(x, gradient * x + c);
    }

    bool isPointOnLine(Coord point) const {
        return gradient * point.x + c == point.y;
    }
};

inline StraightLineEq getEquationOfLine(line_t line) {
    float gradient = static_cast<float>(line.second.y - line.first.y) /
                     static_cast<float>(line.second.x - line.first.x);
    if (std::isnan(gradient)) {
        gradient = 0.0f;
    }
    float c = -gradient * line.first.x + line.first.y;
    return StraightLineEq(gradient, c);
}

} //namespace bbvision
