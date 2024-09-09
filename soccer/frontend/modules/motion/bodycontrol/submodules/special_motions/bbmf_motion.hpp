#pragma once
#include <bodycontrol/internals/submodule.h>
#include <framework/joints/joints.hpp>
#include <bodycontrol/utils/bbmf.h>

class BBMFMotion : public SubModule {

public:
    BBMFMotion(const std::string &filename, const bool deactivate = true, const bool useLinearInterpol = false,
            const bool useAnkleBalancer = false)
      : bbmf(fromFile, filename, useLinearInterpol, useAnkleBalancer, true), deactivate(deactivate) {}

    void setup(Setup) override {
        bbmf.setup();
    }

    SubModuleReturnValue step(BodyBlackboard *bb) override {
        const auto &gyro{bb->sensors.imu.gyroscope};
        bbmf.step(bb->timestamp_ms, bb->actuators, bb->sensors, gyro.x(), gyro.y());

        if (!bbmf.isActive())
            return (deactivate) ? SubModuleReturnValue::DEACTIVATE_ME : SubModuleReturnValue::MOTION_STABLE;

        return SubModuleReturnValue::MOTION_UNSTABLE;
    }

    void reset() override { bbmf.reset(); }

private:
    BBMF bbmf;
    bool deactivate;
};
