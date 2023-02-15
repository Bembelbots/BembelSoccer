#include "config.h"
#include "behavior.h"

#include <behavior/standalone/log.h>

#include <behavior/behavior.h>
#include <behavior/reactivewalk/reactivewalkblackboard.h>

// setGlobalTimeFromSimulation
#include <core/util/clock_simulator.h>


using namespace stab;

Behavior::Behavior(RobotRole role)
  : rwcalib(new ReactiveWalkBlackboard), behavior(new BEHAVE_PRIVATE::Behavior), role(role) {
}

Behavior::Behavior(Behavior &&other)
  : rwcalib(std::move(other.rwcalib))
  , behavior(std::move(other.behavior))
  , role(other.role)
  , log(std::move(other.log)) {
}

Behavior::~Behavior() {}

Behavior &Behavior::operator=(Behavior &&other) {
    rwcalib = std::move(other.rwcalib);
    behavior = std::move(other.behavior);
    output = other.output;
    role = other.role;
    log = std::move(other.log);

    return *this;
}

bool Behavior::doDebugRequest(const BBSetRequest &request) {
    try {
        auto it = behavior->_set_funcs.find(request.name);
        it->second(request.value);
    } catch (...) {
        return false;
    }
    return true;
}

Output &Behavior::execute(const Input &input) {
    Log::instance().setActiveRobot(std::to_string(*input.botid));
    setGlobalTimeFromSimulation(input.timeMs);

    update(input);

    behavior->execute({
        BEHAVE_PRIVATE::Behavior::OptionInfos::getOption(
            behavior->behavior_root.c_str()
        )
    });

    LOG_DEBUG_EVERY_N(30) << "Walk vector:"
                          << DirectedCoord{behavior->walk_x, behavior->walk_y, Rad{behavior->walk_theta}};

    log << Log::instance().flush();

    output = accumulateOutput();

    return output;
}

void Behavior::update(const Input &input) {

    BEHAVE_PRIVATE::Behavior &b = *behavior;

    // TODO this not needed anymore. behavior can run as a module now, so just set the input with a hook.

    b.time_ms = input.timeMs;
    
    b.game_state = input.gamestate;
    b.set_play = input.setPlay;
    b.whistle    = false;
    b.has_kickoff = input.hasKickoff;
    b.is_penalized = false;
    b.game_phase_penalty_shootout = false;
    b.role_current = role;

    // ball
    Robot thisBot{input.myPos};
    Ball ball{input.ballPosWcs};
    b.ball_rcs_pos = ball.wcs2rcs(thisBot).pos;
    b.ball_age = input.ballAge;
    b.ball_wcs_pos = input.ballPosWcs;
    b.is_nearest_to_ball = nearestToBall(input);
    b._num_detected_robots = input.detectedBots.size();

    b.bot_pos = input.myPos;
    b.bot_conf = 1.f;

    b.bot_id = *input.botid;
    b.ready_pos = input.field->getReadyPose(b.bot_id, b.has_kickoff);

    // TODO: set ball_hit_* fields for goalkeeper

    b.is_standing = true;
    b.fallen_side = FallenSide::NONE;
    b.fallen = false;

    b.playingfield = input.field;

    b.reactivewalk.newTick();
    b.reactivewalk.calibrate(*rwcalib);
    b.reactivewalk.setPlayingField(input.field);
    b.reactivewalk.setBall(input.ballPosWcs);
    b.reactivewalk.setThisRobot(thisBot);

    std::vector<Robot> detectedBots;
    for (const DirectedCoord &bot : input.detectedBots) {
        detectedBots.emplace_back(bot.coord);
    }
    b.reactivewalk.setRobots(detectedBots);

    b.nao_bumper_left = false;
    b.nao_bumper_right = false;

    b.num_active_robots = input.activeBots;

    b.motion_active = (input.isMoving) ? Motion::WALK : Motion::STAND;
    b.standing_up = false;

    b.motion_head_yaw_pos = input.headYaw;
    b.motion_head_pitch_pos = 0.f;
    b.robots = input.myTeam;
}

Output Behavior::accumulateOutput() {
    DirectedCoord walk;

    switch (behavior->bm_type) {
        case Motion::WALK:
            switch (behavior->walk_action) {
                case WalkAction::TIPPLE:
                    break;
                default:
                    walk = {behavior->walk_x, behavior->walk_y, Rad{behavior->walk_theta}};
            }
    default:
        break;
    }

    return Output {
        .walk = walk,
        .bm_type = behavior->bm_type,
        .headYawSpeed = behavior->head_yaw,
        .behaviorGraph = behavior->activationGraph,
        .log = flushLog(),
        .blackboard = dumpBlackboard(),
        .rw_debug_target_dribble = rwcalib->debug_target_dribble,
        .rw_debug_target_approach_wcs = rwcalib->debug_target_approach_wcs,

    };
}

bool Behavior::nearestToBall(const Input &input) const {
    if (input.ballAge >= CONST::max_ball_age) {
        return false;
    }
    float myBallDist = input.ballPosWcs.dist(input.myPos.coord);
    
    bool nearest = true;
    for (size_t i = 0; i < input.myTeam.size(); i++) {

        auto ballAge = input.seenBallsOfMyTeam.at(i).age();
        if (input.myTeam.at(i).id != *input.botid  && ballAge <= CONST::max_ball_age){
            nearest &= myBallDist < input.seenBallsOfMyTeam.at(i).pos.dist(input.myTeam.at(i).pos.coord);
        }
    }
    return nearest;
}

std::string Behavior::flushLog() {
    std::string msgs = log.str();
    log.str(""); // clear content of string stream
    return msgs;
}

std::vector<BlackboardEntry> Behavior::dumpBlackboard() const {
    std::vector<BlackboardEntry> data;

    /* static_assert(std::is_base_of<BehaviorBlackboard, BEHAVE_PRIVATE::Behavior>::value); */
    for (auto &entry : behavior->_get_funcs) {
        data.push_back(BlackboardEntry{
                .name = entry.first,
                .value = entry.second().getValue(),
                .editable = behavior->isEditable(entry.first),
        });
    }

    return data;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
