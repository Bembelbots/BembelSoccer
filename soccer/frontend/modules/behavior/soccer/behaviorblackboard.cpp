#include "standalone/config.h"
#include "behaviorblackboard.h"
#include <framework/logger/logger.h>
#include <framework/benchmark/benchmarking.h>

#include <cmath>

using namespace std;

// BALL MOTION FILTER
// INSIDE_OWN_PENELTY_BOX
// BALL DIST RANK
// GET ENEMY GOAL DIST
// GET SHOT.RCS POSITION
// GET WCS / RCS TARGET ANGLE
// ROBOT POSITION WCS
// LONGEST LINE (dist, angle)
// GAME TIME REMAINING
// DEBUG FACILITY (log options/states...)

// keep some points-of-interest as rcs-coords
// - on each iteration update poi-rcs-coords using mcs!
// - enemy goal, own goal, middle-circle, T-cross
// - maybe also include ball, as worldmodel starts to be useless


BehaviorBlackboard::BehaviorBlackboard() 
    : stab::BlackboardBaseShim("BehaviorBlackboard") {

    _test_start_reset_delay = 3;

    INIT_VAR(cabsl_states, "", "cabsl states called in last behavior tick");

    // gamecontrol
    INIT_ENUM(game_state, GameState::INITIAL, "current game state");
    INIT_ENUM(game_state_led, GameState::INITIAL, "game state displayed on led (may differ from GC state, e.g. due to whistle)");
    INIT_ENUM(set_play, SetPlay::NONE, "current set state: None or different free kicks");
    INIT_VAR_RW(whistle, false, "true if whistle has been detected in set state");
    INIT_VAR(whistle_listen, false, "start/stop whistle processing");
    INIT_VAR(has_kickoff, false, "true -> we have kickoff");
    INIT_ENUM(game_team, TeamColor::BLUE, "our team color");
    INIT_VAR(game_phase_penalty_shootout, false,
             "true -> game is in penalty shootout phase");
    INIT_VAR(is_unstiff, false, "true if robot is unstiffed");
    INIT_VAR(is_sitting, false, "true if robot has been unstiffed and is sitting");
    INIT_VAR(is_penalized, false, "true if robot is currently penalized");
    INIT_VAR(was_penalized, false, "true if robot was penalized recently");
    // testing mode vars
    INIT_VAR_RW(test_mode, false, "true -> 'behavior_root' is replaced");
    // sub test units
    std::vector<std::string> _tmp {"test_remote_walk", "test_walk",  "test_kick",
                                   "test_kick3",       "test_align", "test_avoid",
                                   "test_turnaround", "test_unstiff", "test_headmotion"
                                  };
    INIT_VAR(test_units, _tmp, "available test options");
    // active test
    INIT_VAR_RW(test_active, _tmp.at(0), "currently active test");
    INIT_VAR_RW(test_start, false, "set to true to start active test");
    INIT_ENUM_RW(remote_comm, RemoteMotions::STAND, "current remoteMotion");

    // start options for behaviors
    INIT_VAR_RW(behavior_root, "root", "root-option: main behavior");

    // bot(-id) related
    INIT_VAR(bot_id, -1, "Robot ID");
    INIT_VAR(ready_pos, DirectedCoord(), "Ready pos");

    INIT_VAR_RW(goalkeeper_y_dist_ball_defendable, 0.1f,
                "The minimal y distances between bot position and ball position from which the goalkeeper thinks its standing between the ball and goal");

    // roles
    INIT_ENUM(role_current, RobotRole::NONE, "Current robot role");
    INIT_VAR(num_active_robots, 0,
             "the number of our active robots an field during game");

    // sensors
    INIT_VAR(nao_bumper_left, false, "true -> left bumper pushed");
    INIT_VAR(nao_bumper_right, false, "true -> right bumper pushed");

    // ball/landmark positions
    INIT_VAR(ball_rcs_pos, Coord(1.0f, 1.0f), "Ball position RCS");
    INIT_VAR(ball_age, CONSTANTS::max_ball_age * 2, "Ball age in ms");
    INIT_VAR(is_nearest_to_ball, false, "is Robot nearest to ball");
    INIT_VAR(ball_wcs_pos, Coord(0.f, 0.f), "Ball position WCS");
    INIT_VAR(ball_wcs_pos_before_kick, Coord(), "Ball position WCS");

    INIT_VAR(_num_detected_robots, 0, "number detected robots");



    INIT_VAR(bot_pos, DirectedCoord(), "current wcs robot position");
    INIT_VAR(bot_pos_before_movement, DirectedCoord(), "position before movement");
    INIT_VAR(bot_conf, 1.0f, "current wcs robot position");

    INIT_VAR(ball_hits_baseline, false, "does the ball hit the baseline");
    INIT_VAR(ball_hits_where, 0.f, "where does it hit the baseline (m)");
    INIT_VAR(ball_hits_when, 0.f, "time until the ball hits the baseline (msec)");
    INIT_VAR(ball_is_defendable, false, "should we throw us towards the ball");

    // falling vars (fallen, direction, fall control active)
    INIT_VAR(fallen, false, "timestamp (ms) from fall down, -1 == ok");
    INIT_ENUM(fallen_side, FallenSide::NONE, "fall side of the robot");
    INIT_VAR(fall_control, false, "true -> trying to stand up");

    // active motion data
    INIT_ENUM(motion_active, Motion::NONE, "currently active MotionID");
    INIT_VAR_RW(motion_head_pitch_pos, CONST::initial_pitch, "head pitch pos");
    INIT_VAR_RW(motion_head_yaw_pos, CONST::initial_yaw, "head yaw pos");

    ///// OUTPUT STUFF
    // output led control
    INIT_ENUM(chest_color, RGBColor::OFF, "[output] chest color");
    INIT_ENUM(left_foot_color, RGBColor::OFF, "[output] left foot color");
    INIT_ENUM(right_foot_color, RGBColor::OFF, "[output] right foot color");

    // output body motion control -> PREFIX = bm
    INIT_ENUM(bm_type, Motion::NONE, "[output] motion type");
    INIT_VAR(bm_target_pos, Coord(0.f, 0.f), "[output] target position");
    INIT_VAR(bm_target_angle, Angle(), "[output] target angle");

    // output head motion control  -> PREFIX = hm
    INIT_ENUM(hm_type, HeadMotionType::NONE, "[output] head motion type");
    INIT_VAR(hm_pos, Coord(1, 0), "[output] head motion target");
    INIT_VAR(hm_dir, -1, "[output] current head motion direction");

    // output arm motion control
    INIT_ENUM(arms, ArmsPosition::SIDE, "[output] arm motion type");

    // output raw-walk-engine control vars (speed vector) -> PREFIX = walk
    INIT_VAR_RW(walk_x, 0.0f, "[output] x speed (forward) [-1...+1]");
    INIT_VAR_RW(walk_y, 0.0f, "[output] y speed (sideways) [-1...+1]");
    INIT_VAR_RW(walk_theta, 0.0f, "[output] turning speed (radial) [-1...+1]");

    INIT_VAR_RW(high_stand, false, "[output] position body high/low when standing");
    

    // output raw-head-engine control vars (pitch/yaw speed) -> PREFIX = head
    // Units of these vars are unknown (probably rad).
    // With head_yaw = 0.1, the Nao's head rotates with a speed of about
    // 0.1rad per 100ms
    INIT_VAR(head_pitch, 0.0f, "[output] head pitch speed");
    INIT_VAR(head_yaw, 0.0f, "[output] head yaw speed");

    // output say variable
    INIT_VAR_RW(nao_say, "",
                "[output] this will be said by the nao");   //SET TO INIT_VAR!!

    INIT_ENUM(stiffnessCmd, StiffnessCommand::NONE, "");

    INIT_ENUM(intention, Intention::NONE, "");
}

Intention BehaviorBlackboard::determineIntention(){
     /*
        **INTENTIONS**
        Broadcast behavioural intentions to facilitate team orchestration
    */

    if((game_state == GameState::FINISHED)
            or (game_state == GameState::INITIAL) or is_penalized){
        return Intention::NONE;
    }

    for(std::pair<std::string, std::string> &currentOption : current_options){
        if(currentOption.first == "kick_ball"
            or currentOption.first == "goball_kick"
            or currentOption.first == "instep_kick"
            or currentOption.first == "dribble"
            or currentOption.first == "align"
            or currentOption.first == "block_ball"){
                return Intention::PLAY_BALL;
            }
    }

    /*

    if(role_current == RobotRole::GOALKEEPER){
        return Intention::GOALKEEPER;
    }

    if(role_current == RobotRole::DEFENDER){
        return Intention::DEFENDER;
    }

    */

    return Intention::NONE;
}


// shorthand for logging to info and saying it exactly once
void BehaviorBlackboard::log_say(const std::string &msg) {
    LOG_SAY << msg;
}

BehaviorBlackboard::~BehaviorBlackboard() { }
