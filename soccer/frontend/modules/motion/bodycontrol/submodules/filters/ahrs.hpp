#pragma once
#include "bodycontrol/internals/submodule.h"
#include <ahrs/ahrs.h>

class AHRSFilter : public SubModule {
public:
    SubModuleReturnValue step(BodyBlackboard *bb) override {
        bb->accel <<
            bb->sensors[accXSensor],
            bb->sensors[accYSensor],
            bb->sensors[accZSensor];
        bb->gyro <<
            bb->sensors[gyroXSensor],
            bb->sensors[gyroYSensor],
            bb->sensors[gyroZSensor];

        ahrs.update(bb->gyro, bb->accel);
        bb->bodyAngles = ahrs.getEuler();

        return RUNNING;
    }

private:
    AHRSProvider ahrs;

};
