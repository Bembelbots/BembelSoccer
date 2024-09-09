#pragma once

#include "representations/flatbuffers/types/actuators.h"
#include "representations/flatbuffers/types/lola_names.h"
#include <bodycontrol/internals/submodule.h>
#include <framework/joints/joints.hpp>

#include <framework/logger/logger.h>

#include <bodycontrol/utils/state_machine.h>

class Stiffness : public SubModule {

public:
    using Command = StiffnessCommand;

    Stiffness() {
        unstiff.fill(-1.f);
        zero.fill(0.f);
        warmup.setFrom(zero);

        target.fill(targetStiffness);
        target[JointNames::LHipYawPitch] = targetHipStiffness;
        warmup.setTo(target);
    }

    void setup(Setup s) override { s.cmds->connect<SetStiffness, &Stiffness::setCommand>(this); }

    SubModuleReturnValue step(BodyBlackboard *bb) override { return BC_RUN_STATE_MACHINE(bb); }

    void reset() override { reset_state_machine(); }

    void setCommand(SetStiffness setter) { this->cmd = setter.cmd; }

private:
    static constexpr float startStiffness = 0.f;
    static constexpr float targetStiffness = 0.8f;
    static constexpr float targetHipStiffness = 0.8f;
    static constexpr float stiffnessInterpolationMs = 250;
    static constexpr float fallenInterpolationMs = 250;

    joints::stiffness::All zero;
    joints::stiffness::All unstiff;
    joints::stiffness::All target;
    joints::stiffness::All stiffness;

    joints::Linear<joints::stiffness::All> warmup;

    Command cmd = StiffnessCommand::NONE;

    SubModuleReturnValue BC_STATE_MACHINE(BodyBlackboard *bb) {
        BC_INITIAL_STATE(init, END_SEQUENCE) {
            BC_STATE_INITIALIZATION {
                LOG_DEBUG << "[S] Init";
                startNewWarmup(bb->timestamp_ms, stiffnessInterpolationMs);
            }
            bool done = warmupStiffness(bb);

            cmd = Command::NONE;

            if (done) {
                goto standing;
            }
        }

        BC_STATE(standing, RUNNING) {
            BC_STATE_INITIALIZATION { LOG_DEBUG << "[S] Idle"; }

            if (cmd == Command::FORCE_STIFF) {
                goto forceStiff;
            } else if (cmd == Command::FORCE_UNSTIFF) {
                goto forceUnstiff;
            } else if (bb->qns[IS_FALLING] and bb->activeMotion != Motion::DESIGNER) {
                goto falling;
            }

            cmd = Command::NONE;
        }

        BC_STATE(falling, END_SEQUENCE) {
            BC_STATE_INITIALIZATION { LOG_DEBUG << "[S] Unstiffing all"; }
            if (!bb->qns[IS_FALLING]) {
                goto fallen;
            }

            unstiff.write(bb->actuators);

            cmd = Command::NONE;
        }

        BC_STATE(fallen, END_SEQUENCE) {
            BC_STATE_INITIALIZATION { LOG_DEBUG << "[S] Fallen"; }
            if (cmd == Command::RESTIFF_ALL) {
                cmd = Command::NONE;
                goto warmup;
            }
        }

        BC_STATE(warmup, END_SEQUENCE) {
            BC_STATE_INITIALIZATION {
                LOG_DEBUG << "[S] Restiffing all";
                startNewWarmup(bb->timestamp_ms, fallenInterpolationMs);
            }
            bool done = warmupStiffness(bb);
            cmd = Command::NONE;

            if (done) {
                goto fallen_idle;
            }
        }

        BC_STATE(fallen_idle, RUNNING) {
            BC_STATE_INITIALIZATION { LOG_DEBUG << "[S] Fallen idle"; }

            if (cmd == Command::FORCE_STIFF) {
                goto forceStiff;
            } else if (!bb->prevQns[IS_FALLEN]) {
                goto standing;
            }

            cmd = Command::NONE;
        }

        BC_STATE(forceStiff, RUNNING) {
            BC_STATE_INITIALIZATION { LOG_DEBUG << "[S] Forced stiff"; }

            if (cmd != Command::FORCE_STIFF)
                goto init;

            // FIXME: stiffness should already be set, but without this line,
            // the bot does not stand up properly after unstiff
            target.write(bb->actuators);
        }

        BC_STATE(forceUnstiff, END_SEQUENCE) {
            BC_STATE_INITIALIZATION { LOG_DEBUG << "[S] Forced unstiff"; }

            if (cmd != Command::FORCE_UNSTIFF)
                goto warmup;

            zero.write(bb->actuators);
        }
    }

    void startNewWarmup(int start, int duration) {
        warmup.setStart(start);
        warmup.setDuration(duration);
    }

    bool warmupStiffness(BodyBlackboard *bb) {
        stiffness = warmup.get(bb->timestamp_ms);

        stiffness.write(bb->actuators);

        return warmup.done(bb->timestamp_ms);
    }
};
