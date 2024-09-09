#pragma once

#include "framework/joints/body_v6.h"
#include "lola_names_generated.h"
#include <bodycontrol/internals/submodule.h>

class InitActuators : public SubModule {

public:
    SubModuleReturnValue step(BodyBlackboard *bb) override {
        constexpr auto &limit{joints::CONSTRAINTS};
        joints::pos::All joints(bb->sensors);

        for (const auto &i : bbapi::EnumValuesJointNames())
            joints[i] = std::clamp(joints[i], limit.at(i).min, limit.at(i).max);

        joints.write(bb->actuators);

        return DEACTIVATE_ME;
    }

private:
    joints::pos::All pos;
};
