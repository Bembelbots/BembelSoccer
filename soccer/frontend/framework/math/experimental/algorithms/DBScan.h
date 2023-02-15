#pragma once

#include "../vector2.h"
#include <cassert>

namespace MathToolbox {
class DBScan {
public:
    DBScan();
    static std::vector<int> run(const std::vector<Vector2<float>> &input, float epsilon, size_t minPts);

private:
    static std::vector<int> getNeigbours(const std::vector<Vector2<float>> &input, size_t index, float epsilon);
};
} // namespace MathToolbox
