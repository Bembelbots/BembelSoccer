#pragma once
#include "bodycontrol/internals/submodule.h"
#include <bodycontrol/utils/joints.hpp>
#include <bodycontrol/utils/stiffness_control.hpp>

class HoldPosition : public SubModule {
public:
    HoldPosition()
            : gotValues(false)
            , joints() {
        reset();
    }

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        if (not gotValues) {
            joints.readSensorAngles(bb->sensors);
            gotValues = true;
        }
        joints.writeActuatorAngles(bb->actuators);
        return MOTION_STABLE;
    }

    void reset() override {
        gotValues = false;
    }

private:
    bool gotValues;
    Joints<All> joints;
};
