#include "DBScan.h"

namespace MathToolbox {

DBScan::DBScan() {
}

// https://en.wikipedia.org/wiki/DBSCAN#Algorithm
std::vector<int> DBScan::run(const std::vector<Vector2<float>> &input, float epsilon, size_t minPts) {
    int clusterCounter = 0;

    std::vector<int> result(input.size(), -1);

    for (std::size_t i = 0; i < input.size(); i++) {
        if (result[i] != -1) {
            continue;
        }

        std::vector<int> neighbours = getNeigbours(input, i, epsilon);

        if (neighbours.size() < minPts - 1) {
            // label as noise
            result[i] = 0;
            continue;
        }

        clusterCounter++;
        result[i] = clusterCounter;

        for (std::size_t j = 0; j < neighbours.size(); j++) {
            // check if this point was previously processed
            if (result[neighbours[j]] > 0) {
                continue;
            }

            // else reclassify
            result[neighbours[j]] = clusterCounter;

            std::vector<int> next_neighbours = getNeigbours(input, neighbours[j], epsilon);

            if (next_neighbours.size() >= minPts) {
                neighbours.insert(neighbours.end(), next_neighbours.begin(), next_neighbours.end());
            }
        }
    }

    return result;
}

std::vector<int> DBScan::getNeigbours(const std::vector<Vector2<float>> &input, size_t index, float epsilon) {
    assert(index < input.size());

    std::vector<int> result;

    Vector2<float> vector = input[index];

    for (std::size_t i = 0; i < input.size(); i++) {
        if (i == index) {
            continue;
        }

        float dist = (vector - input[i]).len();

        if (dist <= epsilon) {
            result.push_back(i);
        }
    }

    return result;
}

} // namespace MathToolbox
