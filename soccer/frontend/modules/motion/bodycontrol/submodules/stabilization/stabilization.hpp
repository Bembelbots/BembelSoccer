#pragma once

#include "bodycontrol/internals/submodule.h"

class Stabilization : public SubModule {

public:
    SubModuleReturnValue step(BodyBlackboard * bb) override {
        if(deactivate){
            bb->stabilizationType = StabilizationType::DEACTIVATED;
            return RUNNING;
        }

        float gyroX = bb->sensors[gyroXSensor];
        float gyroY = bb->sensors[gyroYSensor];

        switch(bb->stabilizationType){
            case StabilizationType::ARMS:
                stabilizeUsingArms(bb->actuators.get().data(), gyroX, gyroY);
                break;
            default:
                // do nothing
                break;
        }

        return RUNNING;
    }

private:
    bool deactivate = false;

    void stabilizeUsingArms(float *actuators, const float &angleX, const float &angleY) {
        actuators[rShoulderRollPositionActuator] += angleX * 2.0f;
        actuators[lShoulderRollPositionActuator] -= angleX * 2.0f;
        actuators[rShoulderPitchPositionActuator] -= angleY * 2.0f;
        actuators[lShoulderPitchPositionActuator] -= angleY * 2.0f;
    }

};
