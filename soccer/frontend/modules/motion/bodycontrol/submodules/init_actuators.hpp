#pragma once

#include <bodycontrol/internals/submodule.h>

#include <framework/joints/joints.hpp>


class InitActuators : public SubModule {

public:
    SubModuleReturnValue step(BodyBlackboard *bb) override {
        pos.read(bb->sensors);
        pos.write(bb->actuators);

        return DEACTIVATE_ME;
    }

private:

    joints::pos::All pos;
    
};
