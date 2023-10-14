#pragma once

#include <representations/motion/motion.h>
#include <representations/bembelbots/types.h>
#include <representations/bembelbots/constants.h>
#include <representations/camera/cam_pose_struct.h>
#include <representations/motion/body_state.h>
#include <framework/joints/joints.hpp>
#include <framework/math/coord.h>
#include <framework/math/directed_coord.h>

#include <Eigen/Core>


class BodyBlackboard {
public:
    BodyBlackboard()
        : headPitchLastPos(CONST::initial_pitch) {
    }

    DirectedCoord walkPosition;
    std::array<float, 3> walkMaxSpeed;

    DirectedCoord odometry;

    StabilizationType stabilizationType{StabilizationType::DEACTIVATED};

    Eigen::Vector3f accel;
    Eigen::Vector3f gyro;
    Eigen::Vector3f bodyAngles;

    joints::Actuators actuators;

    bool connected;

    uint32_t timestamp_ms;
    uint32_t tick;
    int64_t lola_timestamp; //< recv. timestamp from lola backend, used to match camera images

    camPose bCamPose, tCamPose;

    joints::Sensors sensors;

    float headYawLastPos = 0.0f;
    float headPitchLastPos;

    FallenSide fallenSide = FallenSide::NONE;
    
    std::array<Eigen::Vector3f, 6> com;

    Motion activeMotion;

    std::bitset<NUM_OF_BODY_QUESTIONS> qns;
    std::bitset<NUM_OF_BODY_QUESTIONS> prevQns;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
