#pragma once

#include <bodycontrol/utils/motionfile.h>
#include <framework/joints/joints.hpp>
#include <framework/logger/logger.h>

#include <unistd.h>
#include <iostream>

class kick_v3_l : public SubModule {
public:

    // bool useLinearInterpol
    kick_v3_l() : mf(fromFile, "kick_v3_l.mf", false) {
        stiffness.fill(0.8f);
    }

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        stiffness.write(bb->actuators);
        mf.step(bb->timestamp_ms, bb->actuators, bb->sensors, bb->gyro(0), bb->gyro(1));

        // LOG_DEBUG_EVERY_N(200) << "KICKING";
        if(not mf.isActive()){
            return SubModuleReturnValue::MOTION_STABLE;
        }

        return SubModuleReturnValue::MOTION_UNSTABLE;
    }

    void reset() override {
        mf.reset();
    }

private:
    MotionFile mf;
    joints::stiffness::All stiffness;
};

class kick_v3_r : public SubModule {
public:

    // bool useLinearInterpol
    kick_v3_r() : mf(fromFile, "kick_v3_r.mf", false) {
        stiffness.fill(0.8f);
    }

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        stiffness.write(bb->actuators);
        mf.step(bb->timestamp_ms, bb->actuators, bb->sensors, bb->gyro(0), bb->gyro(1));

        // LOG_DEBUG_EVERY_N(200) << "KICKING";
        if(not mf.isActive()){
            return SubModuleReturnValue::MOTION_STABLE;
        }

        return SubModuleReturnValue::MOTION_UNSTABLE;
    }

    void reset() override {
        mf.reset();
    }

private:
    MotionFile mf;
    joints::stiffness::All stiffness;
};
