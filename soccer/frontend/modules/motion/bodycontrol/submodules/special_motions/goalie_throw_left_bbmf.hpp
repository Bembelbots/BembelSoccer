#pragma once

#include <bodycontrol/internals/submodule.h>
#include <bodycontrol/utils/bbmf.h>
#include <framework/logger/logger.h>

#include <unistd.h>
#include <iostream>

class GoalieThrowLeftBBMF : public SubModule {
	public:
		GoalieThrowLeftBBMF() : bbmf(fromFile, "GoalieThrowLeft.bbmf"){
		}

		SubModuleReturnValue step(BodyBlackboard * bb) override {
        bbmf.step(bb->timestamp_ms, bb->actuators, bb->sensors, 
                    bb->gyro(0), bb->gyro(1));

        if(not bbmf.isActive()){
            return SubModuleReturnValue::MOTION_STABLE;
        }

        return SubModuleReturnValue::MOTION_UNSTABLE;
    }

    void reset() override {
        bbmf.reset();
    }

private:
	BBMF bbmf;
};
