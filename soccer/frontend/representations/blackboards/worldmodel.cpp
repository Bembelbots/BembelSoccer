#include "worldmodel.h"
#include "../bembelbots/constants.h"
#include <framework/util/assert.h>

WorldModelBlackboard::WorldModelBlackboard()
    : SnapshotableBlackboard("WorldModel") {
        setup();
}

void WorldModelBlackboard::setup() { 
    INIT_VAR(myRobotPoseWcs, {}, "WCS position of current robot");
    INIT_VAR(myBallPoseRcs, {}, "RCS position of last seen ball");
    INIT_VAR(myBallPoseWcs, {}, "WCS position of last seen ball");
    INIT_VAR(iAmNearestToOwnBall, false, "am I?");
    INIT_VAR(iAmNearestToTeamBall, false, "am I?");
    INIT_VAR(myBallHitsBaseline, false, "true, if ball will hit my baseline");
    INIT_VAR(myBallHitsBaselineWhere, 0.0f, "y-coord (left < 0?) in meters relative to me");
    INIT_VAR(myBallHitsBaselineWhen, 0.0f, "time in milli-secs until the ball will hit the baseline");
    INIT_VAR(detectedRobots, {}, "robots that are detected");
    INIT_VAR(allRobots, {}, "all robots including teammates, opponents and robot");
    INIT_VAR(allBallPoseWcs, {}, "all balls seen by teammates and robot");
    INIT_VAR(ballSize, CONST::ball_size, "");
    INIT_VAR(robotSize, CONST::robot_size, "");
    
    INIT_VAR(myBallIsOutlier, false, "true if my ball is an outlier during teamball calculation");

    INIT_VAR(teamball, Ball(), "teamball object of the current teamball");

    INIT_VAR(myRole, (int)RobotRole::NONE, "asigned role by behavior");

    INIT_VAR_RW(teamBallNearestToBallTolerance, 0.01, "tolerance for nearest to ball calculations (2 robots can be nearest to ball)");

    INIT_VAR_RW(teamBallMaxDistance, 1, "max distance of a ball belonging to the teamball from cluster center to ball");

    INIT_VAR_RW(teamBallDBScanMinPts, 2, "minPts parameter for DBScan");    

    INIT_VAR_RW(teamBallConfidenceFalloff, 0.1, "alpha for the e^(-alpha*dist) weight factor in the teamball calculation");
    
    //INIT_VAR(situationBasedBallPositions1, std::vector<Coord>(), "returns possible ball locations based on different situations");

    // easy (haha) control the ball filter
    INIT_VAR_RW(ballFilterMinNrOfValues, 3, "");
    INIT_VAR_RW(ballFilterMaxVelocityViolations, 5, "");
    INIT_VAR_RW(ballFilterMaxAngleViolations, 3, "");
    INIT_VAR_RW(ballFilterVelocityThreshold, 0.75f, "");
    INIT_VAR_RW(ballFilterAngleThreshold, 0.0f, "");
    INIT_VAR_RW(ballFilterMaxTimeInterval, 1.0f, "");
    
    for (size_t x = 0; x < NUM_PLAYERS; x++) {
        Robot *r = &allRobots[x];
        Ball *b = &allBallPoseWcs[x];
        r->id = x;
        r->ball = b;
        b->id = x;
        b->robot = r;
    }
}
