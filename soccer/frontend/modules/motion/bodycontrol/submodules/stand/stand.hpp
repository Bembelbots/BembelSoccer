#pragma once

#include <bodycontrol/internals/submodule.h>

#include <framework/joints/joints.hpp>

#include <framework/joints/special_stances.hpp>
#include <framework/logger/logger.h>

#include <cmath>

#include <bodycontrol/utils/state_machine.h>


class InterpolateToStand : public SubModule {

public:

    SubModuleReturnValue BC_STATE_MACHINE(BodyBlackboard *bb) {
        BC_INITIAL_STATE(close_hips, MOTION_UNSTABLE) {

            BC_STATE_INITIALIZATION {
                motion.setStart(bb->timestamp_ms);
                motion.setDuration(motionTime);

                current.read(bb->sensors);
                motion.setFrom(current);

                motion.setTo(STANDUP_STANCE);
            }

            current = motion.get(bb->timestamp_ms);
            current.write(bb->actuators);

            if (motion.done(bb->timestamp_ms)) {
                // relax joint by accepting current position as target
                current.read(bb->sensors);
                current.write(bb->actuators);

                goto finish;
            }
        }

        BC_STATE(finish, MOTION_STABLE) {
            bb->qns[IS_STANDING] = true;
        }

    }

    SubModuleReturnValue step(BodyBlackboard *bb) {
        return BC_RUN_STATE_MACHINE(bb);
    }

    void reset() {
        reset_state_machine();
    }

private:
    static constexpr float motionTime = 1000;

    joints::Linear<joints::pos::All> motion;

    joints::pos::All current;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
