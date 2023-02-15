#pragma once

#include "definitions.h"

#include "standalone/blackboardbase_shim.h"

#include <representations/motion/body_state.h>
#include <representations/bembelbots/types.h>
#include <representations/spl/spl.h>
#include <representations/teamcomm/types.h>
#include <representations/whistle/commands.h>
#include <representations/worldmodel/definitions.h>
#include <representations/playingfield/playingfield.h>

#include <string>


class BehaviorBlackboard : public stab::BlackboardBaseShim {
public:

    explicit BehaviorBlackboard();

    ~BehaviorBlackboard();

    /************************************************************************/
    /* Behavior variables                                                   */

    // cabsl debug
    MAKE_VAR(std::string, cabsl_states);
    
    //// GAME CONTROLLER VARIABLES
    MAKE_VAR(GameState, game_state);
    MAKE_VAR(GameState, game_state_led);
    MAKE_VAR(SetPlay, set_play);
    MAKE_VAR(bool, whistle);
    MAKE_VAR(bool, whistle_listen);
    MAKE_VAR(bool, has_kickoff);

    MAKE_VAR(TeamColor, game_team);
    MAKE_VAR(bool, is_unstiff);
    MAKE_VAR(bool, is_sitting);
    MAKE_VAR(bool, is_penalized);
    MAKE_VAR(bool, was_penalized);

    MAKE_VAR(bool, game_phase_penalty_shootout);

    //// TESTING MODE ACTIVATION & CONTROL - PREFIX = test
    MAKE_VAR(bool, test_mode);
    MAKE_VAR(std::string, test_active);
    MAKE_VAR(std::vector<std::string>, test_units);
    MAKE_VAR(bool, test_start);

    //// ROOT OPTION VARIABLES
    //// - name of the (paramless) option to be executed for named behavior
    MAKE_VAR(std::string, behavior_root);

    //// ROBOT DEPENDENT VARIABLES
    // unique bot-id [0..n]
    MAKE_VAR(int, bot_id);
    // ready position/angle for this bot(-id) and game-state (WCS)
    MAKE_VAR(DirectedCoord, ready_pos);

    MAKE_VAR(int, ready_pos_dist_state_time);
    MAKE_VAR(bool, ready_pos_turn_left);


    // role variables - PREFIX = role
    MAKE_VAR(RobotRole, role_current);
    // number of active players in game.
    MAKE_VAR(int, num_active_robots);

    MAKE_VAR(float, goalkeeper_y_dist_ball_defendable);

    //// POINT-OF-INTERESTS VARIABLES
    // ball position/confidence (RCS)
    MAKE_VAR(Coord, ball_rcs_pos);
    MAKE_VAR(int, ball_age);
    MAKE_VAR(bool, is_nearest_to_ball);
    MAKE_VAR(DirectedCoord, target_pos);
    // ball wcs
    MAKE_VAR(Coord, ball_wcs_pos);
    MAKE_VAR(Coord, ball_wcs_pos_before_kick);

    MAKE_VAR(int, _num_detected_robots);

    MAKE_VAR(float, area_x1);
    MAKE_VAR(float, area_x2);
    MAKE_VAR(float, area_y1);
    MAKE_VAR(float, area_y2);

    MAKE_VAR(Coord, area_var1);
    MAKE_VAR(Coord, area_var2);
    

    MAKE_VAR(DirectedCoord, bot_pos);
    MAKE_VAR(float, bot_conf);

     //position before movement
    MAKE_VAR(DirectedCoord, bot_pos_before_movement);

    MAKE_VAR(bool, ball_hits_baseline);
    MAKE_VAR(float, ball_hits_where);
    MAKE_VAR(float, ball_hits_when);
    MAKE_VAR(bool, ball_is_defendable);

    // nao sensors - PREFIX=nao
    MAKE_VAR(bool, nao_bumper_left);
    MAKE_VAR(bool, nao_bumper_right);

    //// MOTION RELATED VARIABLES
    // fallen -> true, if bot has fallen down
    MAKE_VAR(bool, is_standing);
    MAKE_VAR(bool, fallen);
    MAKE_VAR(FallenSide, fallen_side);
    MAKE_VAR(bool, fall_control);

    // active motions
    MAKE_VAR(Motion, motion_active);
    // head positions
    MAKE_VAR(float, motion_head_pitch_pos);
    MAKE_VAR(float, motion_head_yaw_pos);
    MAKE_VAR(bool, standing_up);

    //// OUTPUT VARIABLES
    //// - (usually) not set by update() - only from inside the behavior(s)
    //// - applied to the framework through the apply{LEDs,Motion,...}() methods

    // output led control
    MAKE_VAR(RGBColor, chest_color);
    MAKE_VAR(RGBColor, left_foot_color);
    MAKE_VAR(RGBColor, right_foot_color);

    // output body motion control - PREFIX = bm
    MAKE_VAR(Motion, bm_type);
    MAKE_VAR(Coord, bm_target_pos);
    MAKE_VAR(Angle, bm_target_angle);

    // output head motion control - PREFIX = hm
    MAKE_VAR(HeadMotionType, hm_type);
    MAKE_VAR(Coord, hm_pos);
    MAKE_VAR(int, hm_dir);

    // output arm position control
    MAKE_VAR(ArmsPosition, arms);

    // output raw-walk-engine control vars (speeds) [-1...+1] - PREFIX = walk
    MAKE_VAR(WalkAction, walk_action);
    MAKE_VAR(float, walk_x);
    MAKE_VAR(float, walk_y);
    MAKE_VAR(float, walk_theta);

    // Tell walk to position body high or low when standing
    MAKE_VAR(bool, high_stand);

    // output raw-head-engine control vars (speeds) [-1...+1] - PREFIX = head
    MAKE_VAR(float, head_pitch);
    MAKE_VAR(float, head_yaw);

    // say something output var
    MAKE_VAR(std::string, nao_say);

    //remotes commands from nao debug/bembelDbug
    MAKE_VAR(RemoteMotions, remote_comm);

    //const float feet_width = 0.095;
    //const float opt_ball_dist = 0.14; //feet length = 10 cm + offset

    // Behaviour Intentions
    MAKE_VAR(Intention, intention);

    MAKE_VAR(StiffnessCommand, stiffnessCmd);

    std::array<Robot, NUM_PLAYERS> robots;

    int _test_start_reset_delay;

    std::string last_nao_say;

    const PlayingField *playingfield = nullptr;

    std::bitset<NUM_OF_BODY_QUESTIONS> bodyqns; // HACK REMOVE ME

    Intention determineIntention();

    /***************************************************************************/
    /* Utility behavior methods (keep them inline for more performance!)       */

    // shortcut for debugging 'msg' is put into 'nao_say' and 'LOG_INFO'
    void log_say(const std::string &msg);

    //DirectedCoord getnextlineforgoalie();

    std::vector<std::pair<std::string, std::string>> current_options;



protected:

    static constexpr float feet_width = 0.075;
    static constexpr float opt_ball_dist = 0.155; //feet length = 10 cm + offset
};

// vim: set ts=4 sw=4 sts=4 expandtab:
