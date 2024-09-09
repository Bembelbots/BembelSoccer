#pragma once

#include "bodycontrol/internals/submodule.h"

class GroundContact : public SubModule {

public:
    SubModuleReturnValue step(BodyBlackboard * bb) override {
        constexpr float THRESHOLD{.1f};
        const auto &fsr{bb->sensors.fsr};
        // check ground contact
        bool leftGroundContact = false;
        for (const float &f: fsr.left.arr)
            leftGroundContact |= (f > THRESHOLD);

        bool rightGroundContact = false;
        for (const float &f: fsr.right.arr)
            rightGroundContact |= (f > THRESHOLD);

        bb->qns[HAS_GROUND_CONTACT] = (rightGroundContact || leftGroundContact);

        return RUNNING;
    }
};
