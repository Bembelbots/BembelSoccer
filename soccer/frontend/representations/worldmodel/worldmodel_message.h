#pragma once

#include "definitions.h"
#include "../bembelbots/constants.h"
#include "../playingfield/playingfield.h"

#include <array>

struct WorldmodelMessage {
    Ball myBallPoseRcs;
    Ball myBallPoseWcs;
    bool iAmNearestToOwnBall;
    std::vector<Robot> detectedRobots;
    Robot myRobotPoseWcs;
    bool myBallHitsBaseline;
    float myBallHitsBaselineWhere;
    float myBallHitsBaselineWhen;
    std::array<Robot, NUM_PLAYERS> allRobots;
    std::array<Ball, NUM_PLAYERS> allBallPoseWcs;
    float ballSize;
    float robotSize;
};
