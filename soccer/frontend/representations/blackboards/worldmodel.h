#pragma once
#include <array>
#include <framework/blackboard/blackboard.h>
#include <framework/math/coord.h>
#include "../worldmodel/definitions.h"

// here we save the five robots of our team

class WorldModelBlackboard : public SnapshotableBlackboard {
public:
    using SnapshotableBlackboard::SnapshotableBlackboard;
    WorldModelBlackboard();

    void setup() override;

    /* behavior begin */
    MAKE_VAR(Robot, myRobotPoseWcs);
    MAKE_VAR(Ball, myBallPoseRcs);
    MAKE_VAR(Ball, myBallPoseWcs);
    MAKE_VAR(bool, iAmNearestToOwnBall);
    MAKE_VAR(bool, iAmNearestToTeamBall);
    MAKE_VAR(bool, myBallHitsBaseline);        // true if the ball hits the baseline
    MAKE_VAR(float, myBallHitsBaselineWhere);  // position where the ball hits the baseline
    MAKE_VAR(float, myBallHitsBaselineWhen);   // time when the ball hits the baseline in MS
    MAKE_VAR(std::vector<Robot>, detectedRobots);
    MAKE_VAR(robotArray, allRobots);
    MAKE_VAR(ballArray, allBallPoseWcs);
    MAKE_VAR(float, ballSize);
    MAKE_VAR(float, robotSize);
    /* behavior end */
    
    MAKE_VAR(bool, isInsidePenaltybox);        // variable for goalkeeper

    MAKE_VAR(Ball, teamball);

    MAKE_VAR(int, myRole);

    MAKE_VAR(bool, myBallIsOutlier);

    MAKE_VAR(float, teamBallNearestToBallTolerance);

    MAKE_VAR(float, teamBallMaxDistance);

    MAKE_VAR(int, teamBallDBScanMinPts);

    MAKE_VAR(float, teamBallConfidenceFalloff);

    //MAKE_VAR(std::vector<Coord>, situationBasedBallPositions1);

    // These are tuning variables, they are here to change them in the debugger.
    // The filter has to reset sometimes when the ball stops or is reflected.
    // It tests velocity, direction and time intervals and resets on some conditions.
    // See also in <pose/ballmotionfilter.h> where the tuning functions are.

    MAKE_VAR(int, ballFilterMinNrOfValues);          //!< needs a minimal amount of data. else confidence = 0
    MAKE_VAR(int, ballFilterMaxVelocityViolations);  //!< how often do we ignore small velocities
    MAKE_VAR(int, ballFilterMaxAngleViolations);     //!< how often do we ignore false directions
    MAKE_VAR(float, ballFilterVelocityThreshold);    //!< how slow is considered a small velocity that is no velocity
    MAKE_VAR(float, ballFilterAngleThreshold);       //!< what angle is considered not in line with the assumed path
    MAKE_VAR(float, ballFilterMaxTimeInterval);      //!< how old has our data to be to reset the filterx

};
