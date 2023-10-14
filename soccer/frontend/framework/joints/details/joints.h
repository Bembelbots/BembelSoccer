#pragma once

#include <array>
#include "../definitionsBody.h"

namespace joints {
namespace details {

struct Joints {

    float headYaw = 0;
    float headPitch = 0;

    float lShoulderPitch = 0;
    float lShoulderRoll = 0;

    float lElbowYaw = 0;
    float lElbowRoll = 0;

    float lWristYaw = 0;
    float lHand = 0;

    float hipYawPitch = 0;

    float lHipRoll = 0;
    float lHipPitch = 0;
    float lKneePitch = 0;
    float lAnklePitch = 0;
    float lAnkleRoll = 0;

    float rShoulderPitch = 0;
    float rShoulderRoll = 0;

    float rElbowYaw = 0;
    float rElbowRoll = 0;

    float rWristYaw = 0;
    float rHand = 0;

    float rHipRoll = 0;
    float rHipPitch = 0;
    float rKneePitch = 0;
    float rAnklePitch = 0;
    float rAnkleRoll = 0;

    std::array<float, NR_OF_JOINTS> arr() const {
        std::array<float, NR_OF_JOINTS> val;

        val[HEAD_YAW]         = headYaw;
        val[HEAD_PITCH]       = headPitch;
        val[L_SHOULDER_PITCH] = lShoulderPitch;
        val[L_SHOULDER_ROLL]  = lShoulderRoll;
        val[L_ELBOW_YAW]      = lElbowYaw;
        val[L_ELBOW_ROLL]     = lElbowRoll;
        val[L_WRIST_YAW]      = lWristYaw;
        val[L_HAND]           = lHand;
        val[L_HIP_YAW_PITCH]  = hipYawPitch;
        val[L_HIP_ROLL]       = lHipRoll;
        val[L_HIP_PITCH]      = lHipPitch;
        val[L_KNEE_PITCH]     = lKneePitch;
        val[L_ANKLE_PITCH]    = lAnklePitch;
        val[L_ANKLE_ROLL]     = lAnkleRoll;

        val[R_SHOULDER_PITCH] = rShoulderPitch;
        val[R_SHOULDER_ROLL]  = rShoulderRoll;
        val[R_ELBOW_YAW]      = rElbowYaw;
        val[R_ELBOW_ROLL]     = rElbowRoll;
        val[R_WRIST_YAW]      = rWristYaw;
        val[R_HAND]           = rHand;
        val[R_HIP_ROLL]       = rHipRoll;
        val[R_HIP_PITCH]      = rHipPitch;
        val[R_KNEE_PITCH]     = rKneePitch;
        val[R_ANKLE_PITCH]    = rAnklePitch;
        val[R_ANKLE_ROLL]     = rAnkleRoll;

        return val;
    }

};

} // namespace details
} // namespace joints

// vim: set ts=4 sw=4 sts=4 expandtab:
