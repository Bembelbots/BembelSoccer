#pragma once

#include <algorithm>
#include <cmath>

#include <framework/math/experimental/algorithms/DBScan.h>
#include <framework/math/experimental/vector2.h>

#include <representations/worldmodel/definitions.h>
#include <representations/bembelbots/constants.h>
#include <representations/bembelbots/types.h>

class TeamBall {
public:
    TeamBall(int robotID);
    Ball getCurrentTeamBall();
    void update(std::array<Ball, NUM_PLAYERS> balls, std::array<Robot, NUM_PLAYERS> robots,
            float teamBallConfidenceFalloff, float teamBallNearestToBallTolerance, float teamBallDBScanEpsilon,
            int teamBallDBScanMinPts);
    bool isOutlier();
    bool isNearestToOwnBall();
    bool isNearestToTeamBall();
    Ball getLastSeenBall();

private:
    std::array<Ball, NUM_PLAYERS> m_currentBalls;
    std::array<Robot, NUM_PLAYERS> m_currentRobots;
    std::vector<int> m_currentClusters;
    MathToolbox::DBScan m_dbscan;
    int m_maxCluster{0};
    float m_teamBallConfidenceFalloff{0};
    float m_teamBallNearestToBallTolerance{0};
    int m_robotID;
    TimestampMs lastDebugToConsole{-1};
};
