#pragma once
#include "bodycontrol/internals/submodule.h"
#include <ahrs/ahrs.h>

class AHRSFilter : public SubModule {
public:
    SubModuleReturnValue step(BodyBlackboard *bb) override {
        ahrs.update(bb->sensors.imu);
        bb->bodyAngles = ahrs.getEuler();

        return RUNNING;
    }

private:
    AHRSProvider ahrs;

};
