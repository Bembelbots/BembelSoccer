#pragma once

#include <framework/math/directed_coord.h>
#include <representations/vision/visiondefinitions.h>
#include <representations/playingfield/playingfield.h>
#include <vector>



class HypothesesGenerator{
public:
    HypothesesGenerator(const PlayingField *field = nullptr);
    ~HypothesesGenerator() {};

    std::pair<std::vector<DirectedCoord>,int> createHypotheses(const std::vector<VisionResult> &vrs);

    float max_dist = 0.3;
    float max_angledist = 0.15; //0.174 rad = ca. 10 grad

private:
    const PlayingField *playingfield;
    void circlePreprocessing(VisionResult &circle, const std::vector<VisionResult> &vrs);
    std::vector<DirectedCoord> createHypothesesOfALandmark(const VisionResult &vr);
    std::pair<std::vector<DirectedCoord>,int> uniteHypothesesOfMultipleLandmarks(const std::vector<std::vector<DirectedCoord>> &hypos);

};
