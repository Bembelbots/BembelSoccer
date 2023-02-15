#include "behaviorcontrol.h"

#include <framework/util/assert.h>
#include <representations/blackboards/settings.h>
#include <framework/benchmark/benchmarking.h>

using BEHAVE_PRIVATE::BehaviorControl;

BehaviorControl::~BehaviorControl() {
}

void BehaviorControl::connect(rt::Linker &link) {
    link.name = "Behavior";

    link(settings);
    link(worldBb);
    link(gamecontrolBb);
    link(body);
    link(playingField);
    link(whistle);

//    link(dynamicRole);
    link(bodyCmds);
    link(whistleCmds);
}

void BehaviorControl::setup() {
    _myBehavior = std::make_shared<Behavior>();
}

void BehaviorControl::process() {
    microTime beh_start = getMicroTime();
    run(beh_start / 1000);
}

/**
 * FIXME Why use microTime instead of TimestampMs as type, when input should be in ms?
 */
void BehaviorControl::run(microTime time_ms) {

    // frame start time
    _myBehavior->time_ms = time_ms;

    _myBehavior->bm_type = Motion::STAND;

    // update BehaviorBlackboard from based on other BBs
    START_TIMER("behavior.update");
    updateBehavior();
    STOP_TIMER;

    // short-hand to get option-by-name
    auto getOpt = &Behavior::OptionInfos::getOption;

    roots.clear();
    roots.push_back(getOpt(_myBehavior->behavior_root.c_str()));

    // execute behavior root option
    START_TIMER("behavior.execute");
    _myBehavior->execute(roots);
    STOP_TIMER;

    // apply leds
    START_TIMER("behavior.apply.leds");
    applyLEDs();
    STOP_TIMER;
    // apply motion
    START_TIMER("behavior.apply.motion");
    applyMotion();
    STOP_TIMER;
    // apply misc
    START_TIMER("behavior.apply.misc");
    applyMisc();
    STOP_TIMER;
}

void BehaviorControl::updateBehavior() {
    auto lock = _myBehavior->scopedLock();
    Behavior *bh = _myBehavior.get();
    auto &gc = *gamecontrolBb;
    auto &world = *worldBb;

    // reset body motion output var
    bh->bm_type = Motion::NONE;

    bh->stiffnessCmd = StiffnessCommand::NONE;

    bh->playingfield = playingField;

    // GAME-CONTROLLER (read-only)
    bh->game_team = gc->teamColor;
    bh->game_state = gc->gameState;
    bh->game_state_led = gc->gameState;
    for (const auto &e: whistle.fetch())
        bh->whistle |= e.found;
    bh->has_kickoff = gc->kickoff;
    bh->set_play = gc->setPlay;
    bh->is_unstiff = gc->unstiff;
    bh->is_penalized = (gc->penalty != Penalty::NONE);
    bh->game_phase_penalty_shootout = settings->isPenaltyShootout;
    bh->role_current = settings->role;
    // testing related (reset to false)
    if (bh->test_start) {
        bh->_test_start_reset_delay--;
        if (!bh->_test_start_reset_delay) {
            bh->_test_start_reset_delay = 3;
            bh->test_start = false;
        }
    }

    // ball position update (read-only)
    auto b = world->myBallPoseRcs;
    bh->ball_rcs_pos = b.pos;
    bh->ball_age = static_cast<int>(b.age());
    auto b2 = world->myBallPoseWcs;
    bh->ball_wcs_pos = b2.pos;
    bh->is_nearest_to_ball = world->iAmNearestToOwnBall;
    bh->_num_detected_robots = world->detectedRobots.size();

    Robot r = world->myRobotPoseWcs;
    if ((r.GTtimestamp > 0) && ((getTimestampMs() - r.GTtimestamp) < 5000)) {
        bh->bot_pos = r.GTpos;
    } else {
        bh->bot_pos = r.pos;
        bh->bot_conf = r.confidence;
    }

    // bot(-id) related (read-only)
    bh->bot_id = settings->id;
    bh->ready_pos = playingField->getReadyPose(bh->bot_id, bh->has_kickoff);

    // ball motion filter vars
    bh->ball_hits_baseline = world->myBallHitsBaseline;
    bh->ball_hits_where = world->myBallHitsBaselineWhere;
    // ball hits when should deliver delta!!!!
    bh->ball_hits_when = world->myBallHitsBaselineWhen;
    bh->ball_is_defendable = (bh->ball_hits_baseline && fabsf(bh->ball_hits_where) > 0.1f &&
                              fabsf(bh->ball_hits_where) < 1.0f && bh->ball_hits_when < 2000.f);

    bh->is_standing = body->qns[IS_STANDING];
    bh->fallen_side = body->fallenSide;
    bh->fallen = body->qns[IS_FALLEN];

    bh->nao_bumper_left = body->qns[LEFT_BUMPER_PRESSED];
    bh->nao_bumper_right = body->qns[RIGHT_BUMPER_PRESSED];

    // set number of active players
    bh->num_active_robots = 0;
    for (auto robot : world->allRobots) {
        bh->num_active_robots += (robot.active ? 1 : 0);
    }

    bh->motion_active = body->activeMotion;
    bh->standing_up =
            (bh->motion_active == Motion::STAND_UP_FROM_FRONT || bh->motion_active == Motion::STAND_UP_FROM_BACK);

    bh->motion_head_yaw_pos = body->lastHeadYaw;
    bh->motion_head_pitch_pos = body->lastHeadPitch;
    RobotRole myRole = RobotRole::NONE;

    bh->robots = world->allRobots;
    //dynamicRole->value = (int)myRole;

    bh->bodyqns = body->qns;
}

void BehaviorControl::applyLEDs() {
    Behavior *bh = _myBehavior.get();

    bodyCmds.enqueue<SetLeds>(LED::CHEST, int(bh->chest_color));
    bodyCmds.enqueue<SetLeds>(LED::LEFT_FOOT, int(bh->left_foot_color));
    bodyCmds.enqueue<SetLeds>(LED::RIGHT_FOOT, int(bh->right_foot_color));

    if (bh->ball_age < 30) {
        bodyCmds.enqueue<SetLeds>(LED::LEFT_EYE, 0x00FF00);
        bodyCmds.enqueue<SetLeds>(LED::BRAIN_LEFT, 0xFFFFFF);
    } else {
        bodyCmds.enqueue<SetLeds>(LED::LEFT_EYE, 0xFF0000);
        bodyCmds.enqueue<SetLeds>(LED::BRAIN_LEFT, 0x00000);
    }
    if (bh->is_nearest_to_ball) {
        bodyCmds.enqueue<SetLeds>(LED::RIGHT_EYE_LEFT, 0xFFFF00);
        bodyCmds.enqueue<SetLeds>(LED::BRAIN_RIGHT, 0xFFFFFF);
    } else {
        bodyCmds.enqueue<SetLeds>(LED::RIGHT_EYE, 0xFF0000);
        bodyCmds.enqueue<SetLeds>(LED::BRAIN_RIGHT, 0x00000);
    }
    if (bh->_num_detected_robots > 0) {
        bodyCmds.enqueue<SetLeds>(LED::RIGHT_EYE_RIGHT, 0x00FFFF);
    }
}

void BehaviorControl::applyMotion() {
    Behavior *bh = _myBehavior.get();

    bodyCmds.enqueue<SetHeadMotion>(bh->hm_type);
    if(bh->hm_type == HeadMotionType::BALL){
        bodyCmds.enqueue<SetHeadLookRCS>(bh->hm_pos);
    }
    

    if (bh->stiffnessCmd != StiffnessCommand::NONE) {
        bodyCmds.enqueue<SetStiffness>(bh->stiffnessCmd);
    }

    DirectedCoord walkV{bh->walk_x, bh->walk_y, Rad{bh->walk_theta}};
    switch (bh->bm_type) {
        case Motion::NONE:
            // do nothing
            break;
        case Motion::WALK:
            switch (bh->walk_action) {
                case WalkAction::TIPPLE:
                    bodyCmds.enqueue<Tipple>();
                    break;
                default:
                    bodyCmds.enqueue<WalkMotion>(walkV);
                    break;
            }
            break;
        case Motion::INTERPOLATE_TO_STAND:
            bodyCmds.enqueue<DoMotion>(Motion::INTERPOLATE_TO_STAND);
            break;
        case Motion::STAND:
            if (bh->high_stand) {
                bodyCmds.enqueue<HighStand>();
            } else {
                bodyCmds.enqueue<StopWalking>();
            }
            break;

        default:
            bodyCmds.enqueue<DoMotion>(bh->bm_type);
    }
}

// apply misc stuff
void BehaviorControl::applyMisc() {
    Behavior *bh = _myBehavior.get();

    START_TIMER("behavior.apply.misc.intention");
    _myBehavior->intention = _myBehavior->determineIntention();
    STOP_TIMER;

    START_TIMER("behavior.apply.misc.whistlecmd");
    if (bh->whistle_listen)
        whistleCmds.enqueue<WhistleStart>();
    else
        whistleCmds.enqueue<WhistleStop>();
    STOP_TIMER;
}

// vim: set ts=4 sw=4 sts=4 expandtab:

