/**
 * @author Module owner: Bembelbots Frankfurt, holgerf
 *
 * $Id$
 *
 */

#include <iomanip>
#include <climits>

#include "worldmodel.h"

#include <framework/logger/logger.h>
#include <representations/playingfield/playingfield.h>
//#include <core/messages/dynamic_role_message.h>
#include <framework/util/assert.h>

/// log a wm snapsnot every ... sec
static const int snapshotIntervalS = 3;

WorldModel::WorldModel()
   : _lastWmDebugToConsole(0) {

}

void WorldModel::connect(rt::Linker &link) {
    link.name = "Worldmodel";
    link(settings);
    link(playingfield);
    link(loca);
    link(visionResults);
    link(teamMessage);
    link(body);
    link(gamecontrol);
    link(output);
}

void WorldModel::setup() {
    teamball = std::make_shared<TeamBall>(settings->id);
    goaliballMotionFilter = std::make_shared<JonathansBallMotionFilter>();
    expBallposFilter = std::make_shared<BallFilter>();
    robot = &world.allRobots[settings->id];
    robot->role = settings->role;
}

void WorldModel::process() {
    auto lock = world.scopedLock();

    for(TeamMessage &message : teamMessage.fetch()) {
        world.allRobots[message.robot.id] = message.robot;
        if (message.ball)
            world.allBallPoseWcs[message.ball->id] = *message.ball;
    } 

    updateRobotPose(); 

    TimestampMs now = getTimestampMs();
    robot->timestamp = now;
    
    // set my active state to my penalty mode
    robot->active = ((*gamecontrol)->penalty == Penalty::NONE);

    useVisionResults(robot->pos);

    useBodyStiffnessforObstacleDetection();

    updateDetectedRobots();

    updateTeamBall();

    // debug current position every 20 second
    static constexpr int interval{20000};
    if ((now - _lastWmDebugToConsole) > interval) {
        LOG_INFO << "current Worldmodel debug: ";
        LOG_INFO << "team id " << settings->teamNumber;

        for (size_t i = 0; i < NUM_PLAYERS; ++i) {
            if ((getTimestampMs() - world.allRobots[i].timestamp) > 2*interval)
                continue;

            LOG_DEBUG << "  #" << i << ": " << world.allRobots[i];
            LOG_DEBUG << "  #" << i << ": " << world.allBallPoseWcs[i];
        }

        LOG_INFO << "TeamBall: " << world.teamball;
        _lastWmDebugToConsole = now;
    }

    //situationBasedBallPositions1 = situationBasedBallPositions();
    *output = world;
}

/*
    Situation           Possible Ball Positions
    Set State           Ball in Middle
    Kick-In             Over Sideline Left, Over Sideline Right
    Corner Kick         Corner Left, Corner Right
    Goal Free Kick      Penalty Box Side Left, Penalty Box Side Right
    Pushing Free Kick   Ball Remains in Place
*/
/*
std::vector<Coord> WorldModel::situationBasedBallPositions(){
    //vector<Coord> coordinates;
    GameState gs = _gamecontrol->gameState;
    bool isCornerKick = _gamecontrol->setPlay==SetPlay::CORNER_KICK;
    bool isGoalFreeKick = _gamecontrol->setPlay==SetPlay::GOAL_KICK;
    if (gs == GameState::SET) 
        return {Coord(0,0)};
    //if (gs == Kick_in) return Coord(); //Over SideLine Left, Over Sideline Right
    if (isCornerKick){
        auto a = playingField->getCorner();
        return std::vector<Coord>(a.begin(), a.end()); //returns 4 corners of the field
    }
    if (isGoalFreeKick){
        auto a = playingField->getGoalSpot();
        return std::vector<Coord>(a.begin(), a.end()); //returns 4 Goal Spots
    }
    return {Coord(0,0)}; //Ball Remains in Place
}
*/

void WorldModel::insertDetectedRobot(Robot newRobot){
    bool replace_old_robot = false;
    
    for (auto &oldRobot : world.detectedRobots){
        //check if robot is an old robot(is in radius of old robot)
        if (newRobot.pos.coord.dist(oldRobot.pos.coord)< CONSTANTS::robot_radius+0.1){
            //if yes replace, if not add
           oldRobot = newRobot; 
           replace_old_robot = true;
           break;
        }
    }

    if (!replace_old_robot){
        world.detectedRobots.push_back(newRobot);
    }       
}

void WorldModel::updateDetectedRobots() {
    auto timestamp = getTimestampMs();
    //delete robots that are too old
    for (uint i= 0;  i < world.detectedRobots.size(); ){
        auto age = timestamp - world.detectedRobots.at(i).timestamp;  
        if (age > CONSTANTS::max_robot_age){
            world.detectedRobots.erase(world.detectedRobots.begin()+i);
        }
        else{
            i++;
        }
    }  
}

void WorldModel::runGoaliballMotionFilter(Coord ball_rcs, Ball &ballWcs){

     // update ball filter parameters from blackboard
    goaliballMotionFilter->updateParameters(world.ballFilterMinNrOfValues,
                                  world.ballFilterMaxVelocityViolations,
                                  world.ballFilterMaxAngleViolations,
                                  world.ballFilterVelocityThreshold,
                                  world.ballFilterAngleThreshold,
                                  world.ballFilterMaxTimeInterval);

    // insert into ball motion filter
    goaliballMotionFilter->addPointXY(ball_rcs.x, ball_rcs.y, ballWcs.timestamp);
    ballWcs.motion.x    = goaliballMotionFilter->xVelocity();
    ballWcs.motion.y    = goaliballMotionFilter->yVelocity();
    ballWcs.motionConfidence = goaliballMotionFilter ->confidence;
    
    // insert some additional infos into worldmodel for the goalkeeper
    // _hitsbaselinewhen is a absolute timestamp when the balls arrives
    world.myBallHitsBaseline =  goaliballMotionFilter->hitsBaseline();
    world.myBallHitsBaselineWhere =  goaliballMotionFilter->hitsBaselineWhere();
    world.myBallHitsBaselineWhen =  goaliballMotionFilter->hitsBaselineWhen()-getTimestampMs();

}

void WorldModel::useVisionResults(const DirectedCoord &myPos){
    if ((*gamecontrol)->penalty != Penalty::NONE) 
        return;

    Ball *ball = robot->ball;
    std::vector<Robot> detectedRobots;
    for (auto vs : *visionResults) {
        switch (vs.type) {
            case JSVISION_BALL:
                {
                    Coord ball_rcs = Coord(vs.rcs_x1, vs.rcs_y1);
                    ball->pos = DirectedCoord(ball_rcs, 0_deg).toWCS(myPos).coord; // convert to WCS using current position


                    if (std::abs(ball->pos.y) + 3*BALL_SIZE > playingfield->_widthInsideBounds/2.f) {
                        LOG_DEBUG << "ignoring ball outside field at " << ball->pos;
                        break;
                    }

                    if (robot->role == RobotRole::GOALKEEPER or robot->role == RobotRole::PENALTYGOALIE)
                        runGoaliballMotionFilter(ball_rcs, *ball); 
                    TimestampMs now = getTimestampMs();
                    ball->timestamp = now;
                    world.myBallPoseRcs = ball_rcs;
                    world.myBallPoseWcs = ball->pos;
                }
                break;
            
            case JSVISION_ROBOT:
                {
                    DirectedCoord wcs_pos = DirectedCoord(vs.rcs_x1, vs.rcs_y1, 0_deg).toWCS(myPos);
                    insertDetectedRobot(Robot(wcs_pos.coord));
                }
                break; 
            default:
                break;
        }
    }
}

void WorldModel::updateTeamBall() {
    teamball->update(world.allBallPoseWcs,
            world.allRobots,  
            world.teamBallConfidenceFalloff,
            world.teamBallNearestToBallTolerance,
            (double)world.teamBallMaxDistance,
            world.teamBallDBScanMinPts);
    world.teamball = teamball->getCurrentTeamBall();
    world.iAmNearestToOwnBall = teamball->isNearestToOwnBall();
    world.iAmNearestToTeamBall = teamball->isNearestToTeamBall();
    world.myBallIsOutlier = teamball->isOutlier();
}

void WorldModel::useBodyStiffnessforObstacleDetection(){
    if (body->qns[IS_OBSTACLE_LEFT]) {
        DirectedCoord wcs_pos = DirectedCoord(0, ROBOT_SIZE, 0_deg).toWCS(robot->pos);
        insertDetectedRobot(Robot(wcs_pos.coord));
    }

    if (body->qns[IS_OBSTACLE_RIGHT]) {
        DirectedCoord wcs_pos = DirectedCoord(0, -ROBOT_SIZE, 0_deg).toWCS(robot->pos);
        insertDetectedRobot(Robot(wcs_pos.coord));
    }
}

void WorldModel::updateRobotPose() {
    robot->pos = loca->pose;
    robot->confidence = loca->confidence;

    // add ground truth data, if available and not too old.
    robot->GTtimestamp = loca->gtTimestamp;
    if ((loca->gtTimestamp > 0)
            && ((getTimestampMs() - loca->gtTimestamp) < 5000)) {
        robot->GTpos = loca->gtPosition;
    }

    robot->fallen = body->qns[IS_FALLEN] || body->qns[IS_STANDING_UP];
    world.myRobotPoseWcs = robot->pos;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
