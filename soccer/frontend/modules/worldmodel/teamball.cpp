#include "teamball.h"
#include "representations/bembelbots/constants.h"
#include <framework/math/experimental/vector2.h>
#include <modules/teamcomm/teamcomm.h>

using MathToolbox::DBScan;
using MathToolbox::Vector2;

/*

This class calculates the current teamball position and confidence,
which should be the position the ball is most likely located.

For this, the ball positions seen by team members are clustered using the DBScan algorithm.
The biggest cluster is chosen as a temball candidate.

The teamball position is calculated by weighting the positions seen by the robots
inversely proportional to the distance of the seen ball to the robot, because
the robot positions get more and more inaccurate as the ball is farther away from
the robot.

The teamball confidence is set to the variance of the positions seen by the robots.

*/

TeamBall::TeamBall(int robotID) : m_dbscan(DBScan()), m_robotID(robotID) {
}

/**
 * calculates and returns the current team ball.
 * The team ball will have posConfidence of -1, if no teamball exists, or
 * a positive confidence indicating the variance of the seen positions around the teamball.
 */
Ball TeamBall::getCurrentTeamBall() {
    Ball result;

    if (m_maxCluster == -1) {
        result.id = -1;
        result.posConfidence = -1;

        return result;
    }

    Vector2<float> teamballPos;
    float confidenceSum = 0;
    float variance = 0;
    int clusteredBalls = 0;

    for (std::size_t i = 0; i < m_currentClusters.size(); i++) {
        if (m_currentClusters[i] == m_maxCluster) {
            float distance = m_currentBalls[i].pos.dist(m_currentRobots[i].pos.coord);
            float confidence = expf(-m_teamBallConfidenceFalloff * distance);
            teamballPos += confidence * Vector2<float>(m_currentBalls[i].pos.x, m_currentBalls[i].pos.y);
            confidenceSum += confidence;
            clusteredBalls++;
        }
    }

    if (clusteredBalls < 2) {
        result.id = -1;
        result.posConfidence = -1;

        return result;
    }

    teamballPos /= confidenceSum;

    for (std::size_t i = 0; i < m_currentClusters.size(); i++) {
        if (m_currentClusters[i] == m_maxCluster) {
            variance += powf((Vector2<float>(m_currentBalls[i].pos.x, m_currentBalls[i].pos.y) - teamballPos).len(), 2);
        }
    }

    result.pos.x = (float)teamballPos.x;
    result.pos.y = (float)teamballPos.y;
    result.posConfidence = expf(-0.5 * variance);

    return result;
}

void TeamBall::update(std::array<Ball, NUM_PLAYERS> balls, std::array<Robot, NUM_PLAYERS> robots,
        float teamBallConfidenceFalloff, float teamBallNearestToBallTolerance, float teamBallDBScanEpsilon,
        int teamBallDBScanMinPts) {

    m_currentBalls = balls;
    m_currentRobots = robots;

    m_teamBallConfidenceFalloff = teamBallConfidenceFalloff;
    m_teamBallNearestToBallTolerance = teamBallNearestToBallTolerance;

    std::vector<Vector2<float>> ballpos;
    std::vector<int> robotIDs;

    m_currentClusters.clear();
    m_currentClusters.resize(NUM_PLAYERS, -1);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        auto age = getTimestampMs() - m_currentBalls[i].timestamp;

        if ((m_currentBalls[i].timestamp > 0) and (age < CONSTANTS::max_ball_age)) {
            Vector2<float> pos;

            pos.x = balls[i].pos.x;
            pos.y = balls[i].pos.y;

            ballpos.push_back(pos);

            // we need to store the robot id, as the number of balls seen is not equal to NUM_PLAYERS
            robotIDs.push_back(i);
        }
    }

    if (ballpos.size() < 2) {
        m_maxCluster = -1;
        return;
    }

    std::vector<int> result = m_dbscan.run(ballpos, teamBallDBScanEpsilon, teamBallDBScanMinPts);

    std::vector<int> counter(result.size(), 0);

    for (std::size_t i = 0; i < result.size(); i++) {
        if (result[i] != -1) {
            counter[result[i]]++;
        }
    }

    // find the max element, excluding the first element of counter (0). a ball classified as zero means
    // it was not part of a cluster
    int maxElement = std::distance(counter.begin(), std::max_element(++counter.begin(), counter.end()));

    // go back from indices in the existing balls to robot ids...

    for (std::size_t i = 0; i < result.size(); i++) {
        m_currentClusters[robotIDs[i]] = result[i];
    }

    if (counter[maxElement] == 0) {
        m_maxCluster = -1;
    } else {
        m_maxCluster = maxElement;
    }
}

bool TeamBall::isOutlier() {
    return m_currentClusters[m_robotID] != m_maxCluster || m_maxCluster == -1;
}

bool TeamBall::isNearestToTeamBall() {
    Ball teamBall = getCurrentTeamBall();

    Vector2<float> teamBallPosition(teamBall.pos.x, teamBall.pos.y);
    Vector2<float> position;
    float minLength = 0;
    int minIndex = -1;

    for (int i = 0; i < NUM_PLAYERS; i++) {
        position.x = m_currentRobots[i].pos.coord.x;
        position.y = m_currentRobots[i].pos.coord.y;

        float length = (position - teamBallPosition).len();

        if (length < minLength || minIndex == -1) {
            minLength = length;
            minIndex = i;
        }
    }

    position.x = m_currentRobots[m_robotID].pos.coord.x;
    position.y = m_currentRobots[m_robotID].pos.coord.y;

    float length = (position - teamBallPosition).len();

    return fabsf(length - minLength) < m_teamBallNearestToBallTolerance;
}

bool TeamBall::isNearestToOwnBall() {
    float minLength = 0;
    int minIndex = -1;

    for (int i = 0; i < NUM_PLAYERS; i++) {
        float length = m_currentBalls[i].pos.dist(m_currentRobots[i].pos.coord);
        auto age = getTimestampMs() - m_currentBalls[i].timestamp;
        if ((length < minLength || minIndex == -1) and (m_currentBalls[i].timestamp > 0) and
                (age < 1.5f * TeamComm::defaultBcastIntervalMs)) {
            minLength = length;
            minIndex = i;
        }
    }
    float length = m_currentBalls[m_robotID].pos.dist(m_currentRobots[m_robotID].pos.coord);

    return fabsf(length - minLength) < m_teamBallNearestToBallTolerance;
}

Ball TeamBall::getLastSeenBall() {
    Ball b{m_currentBalls[0]};

    for (const auto &i: m_currentBalls) {
        if (i.timestamp > 0)
            if (i.timestamp > b.timestamp)
                b = i;
    }

    return b;
}
