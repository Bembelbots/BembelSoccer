#pragma once

#include <Eigen/Core>
#include <representations/motion/motion.h>
#include <representations/bembelbots/types.h>
#include <representations/bembelbots/constants.h>
#include <representations/camera/cam_pose_struct.h>
#include <representations/motion/body_state.h>
#include <representations/flatbuffers/types/sensors.h>
#include <representations/flatbuffers/types/actuators.h>
#include <framework/joints/joints.hpp>
#include <framework/math/coord.h>
#include <framework/math/directed_coord.h>

#include <Eigen/Core>


class BodyBlackboard {
public:
    BodyBlackboard()
        : headPitchLastPos(CONST::initial_pitch) {
    }

    uint32_t timestamp_ms{0};
    uint32_t tick{0};
    int64_t lola_timestamp{0}; //< recv. timestamp from lola backend, used to match camera images

    bool connected{false};
    
    bbipc::Sensors sensors;
    bbipc::Actuators *actuators{nullptr};

    Eigen::Vector3f bodyAngles;
    float supportFoot{0};
    Motion activeMotion{Motion::NONE};

    DirectedCoord odometry;
    DirectedCoord walkPosition;
    std::array<float, 3> walkMaxSpeed{};

    StabilizationType stabilizationType{StabilizationType::DEACTIVATED};

    CamPose bCamPose, tCamPose;

    float headYawLastPos{0};
    float headPitchLastPos{0};

    FallenSide fallenSide{FallenSide::NONE};
    
    std::array<Eigen::Vector3f, 6> com;

    std::bitset<NUM_OF_BODY_QUESTIONS> qns;
    std::bitset<NUM_OF_BODY_QUESTIONS> prevQns;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
