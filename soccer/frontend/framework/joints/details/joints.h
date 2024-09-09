#pragma once

#include "lola_names_generated.h"
#include <array>
#include <representations/flatbuffers/types/lola_names.h>

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

    float hipYawPitch = 0;

    float lHipRoll = 0;
    float lHipPitch = 0;
    float lKneePitch = 0;
    float lAnklePitch = 0;
    float lAnkleRoll = 0;

    float rHipRoll = 0;
    float rHipPitch = 0;
    float rKneePitch = 0;
    float rAnklePitch = 0;
    float rAnkleRoll = 0;

    float rShoulderPitch = 0;
    float rShoulderRoll = 0;

    float rElbowYaw = 0;
    float rElbowRoll = 0;
    float rWristYaw = 0;

    float lHand = 0;
    float rHand = 0;

    std::array<float, LOLA_NUMBER_OF_JOINTS> arr() const {
        std::array<float, LOLA_NUMBER_OF_JOINTS> val;

        val[static_cast<int>(JointNames::HeadYaw)]        = headYaw;
        val[static_cast<int>(JointNames::HeadPitch)]      = headPitch;

        val[static_cast<int>(JointNames::LShoulderPitch)] = lShoulderPitch;
        val[static_cast<int>(JointNames::LShoulderRoll)]  = lShoulderRoll;
        val[static_cast<int>(JointNames::LElbowYaw)]      = lElbowYaw;
        val[static_cast<int>(JointNames::LElbowRoll)]     = lElbowRoll;
        val[static_cast<int>(JointNames::LWristYaw)]      = lWristYaw;

        val[static_cast<int>(JointNames::LHipYawPitch)]   = hipYawPitch;

        val[static_cast<int>(JointNames::LHipRoll)]       = lHipRoll;
        val[static_cast<int>(JointNames::LHipPitch)]      = lHipPitch;
        val[static_cast<int>(JointNames::LKneePitch)]     = lKneePitch;
        val[static_cast<int>(JointNames::LAnklePitch)]    = lAnklePitch;
        val[static_cast<int>(JointNames::LAnkleRoll)]     = lAnkleRoll;

        val[static_cast<int>(JointNames::RHipRoll)]       = rHipRoll;
        val[static_cast<int>(JointNames::RHipPitch)]      = rHipPitch;
        val[static_cast<int>(JointNames::RKneePitch)]     = rKneePitch;
        val[static_cast<int>(JointNames::RAnklePitch)]    = rAnklePitch;
        val[static_cast<int>(JointNames::RAnkleRoll)]     = rAnkleRoll;

        val[static_cast<int>(JointNames::RShoulderPitch)] = rShoulderPitch;
        val[static_cast<int>(JointNames::RShoulderRoll)]  = rShoulderRoll;
        val[static_cast<int>(JointNames::RElbowYaw)]      = rElbowYaw;
        val[static_cast<int>(JointNames::RElbowRoll)]     = rElbowRoll;
        val[static_cast<int>(JointNames::RWristYaw)]      = rWristYaw;
       
        val[static_cast<int>(JointNames::LHand)]          = lHand;
        val[static_cast<int>(JointNames::RHand)]          = rHand;

        return val;
    }

};

} // namespace details
} // namespace joints

// vim: set ts=4 sw=4 sts=4 expandtab:
