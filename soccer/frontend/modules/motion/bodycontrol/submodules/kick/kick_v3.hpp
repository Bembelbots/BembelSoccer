#pragma once

#include "bodycontrol/internals/submodule.h"
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
        const auto &g{bb->sensors.imu.gyroscope};
        mf.step(bb->timestamp_ms, bb->actuators, bb->sensors, g.x(), g.y());

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
        const auto &g{bb->sensors.imu.gyroscope};
        mf.step(bb->timestamp_ms, bb->actuators, bb->sensors, g.x(), g.y());

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
