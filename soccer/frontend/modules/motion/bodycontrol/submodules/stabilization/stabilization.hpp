#pragma once

#include "bodycontrol/internals/submodule.h"
#include "framework/joints/joints.hpp"
#include "representations/flatbuffers/types/actuators.h"
#include <Eigen/src/Core/Matrix.h>

class Stabilization : public SubModule {

public:
    SubModuleReturnValue step(BodyBlackboard *bb) override {
        switch (bb->stabilizationType) {
            case StabilizationType::ARMS:
                stabilizeUsingArms(bb->actuators, bb->sensors.imu.gyroscope);
                break;
            default: // do nothing
                break;
        }

        return RUNNING;
    }

private:
    void stabilizeUsingArms(bbipc::Actuators *actuators, const Eigen::Vector3f &gyro) {
        joints::pos::Arms arms;
        arms.read(actuators);
        arms[JointNames::LShoulderRoll] -= gyro.x() * 2.0f;
        arms[JointNames::RShoulderRoll] += gyro.x() * 2.0f;
        arms[JointNames::LShoulderPitch] -= gyro.y() * 2.0f;
        arms[JointNames::RShoulderPitch] -= gyro.y() * 2.0f;
        arms.write(actuators);
    }
};
