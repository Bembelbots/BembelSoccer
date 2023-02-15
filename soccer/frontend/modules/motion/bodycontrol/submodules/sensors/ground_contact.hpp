#pragma once

#include "bodycontrol/internals/submodule.h"

class GroundContact : public SubModule {

public:
    SubModuleReturnValue step(BodyBlackboard * bb) override {
        // check ground contact
        bool leftGroundContact = false;
        for (int i = lFSRFrontLeftSensor; i <= lFSRRearRightSensor; ++i) {
            if (bb->sensors[i] > 0.1f) {
                leftGroundContact = true;
                break;
            }
        }

        bool rightGroundContact = false;
        for (int i = rFSRFrontLeftSensor; i <= rFSRRearRightSensor; ++i) {
            if (bb->sensors[i] > 0.1f) {
                rightGroundContact = true;
                break;
            }
        }

        bb->qns[HAS_GROUND_CONTACT] = (rightGroundContact || leftGroundContact);

        return RUNNING;
    }

};
