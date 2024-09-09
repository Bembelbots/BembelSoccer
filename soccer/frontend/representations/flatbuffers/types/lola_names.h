#pragma once

#include <string_view>
#include <unordered_map>

#include <lola_names_generated.h>

using bbapi::JointNames;

static constexpr uint8_t LOLA_NUMBER_OF_JOINTS{static_cast<uint8_t>(bbapi::JointNames::MAX) + 1};

constexpr unsigned int operator<<(const int i, const bbapi::JointNames j) {
    return i << static_cast<int>(j);
}

static const std::unordered_map<JointNames, std::string_view> jointNamesTTS{
    {JointNames::HeadPitch,       "Head Pitch"},
    {JointNames::HeadYaw,         "Head Yaw"},
    {JointNames::LAnklePitch,     "Left Ankle Pitch"},
    {JointNames::LAnkleRoll,      "Left Ankle Roll"},
    {JointNames::LElbowRoll,      "Left Elbow Roll"},
    {JointNames::LElbowYaw,       "Left Elbow Yaw"},
    {JointNames::LHipPitch,       "Left Hip Pitch"},
    {JointNames::LHipRoll,        "Left Hip Roll"},
    {JointNames::LHipYawPitch,    "Hip"},
    {JointNames::LKneePitch,      "Left Knee Pitch"},
    {JointNames::LShoulderPitch,  "Left Shoulder Pitch"},
    {JointNames::LShoulderRoll,   "Left Shoulder Roll"},
    {JointNames::RAnklePitch,     "Right Ankle Pitch"},
    {JointNames::RAnkleRoll,      "Right Ankle Roll"},
    {JointNames::RElbowRoll,      "Right Elbow Roll"},
    {JointNames::RElbowYaw,       "Right Elbow Yaw"},
    {JointNames::RHipPitch,       "Right Hip Pitch"},
    {JointNames::RHipRoll,        "Right Hip Roll"},
    {JointNames::RKneePitch,      "Right Knee Pitch"},
    {JointNames::RShoulderPitch,  "Right Shoulder Pitch"},
    {JointNames::RShoulderRoll,   "Right Shoulder Roll"},
    {JointNames::LHand,           "Left Hand"},
    {JointNames::RHand,           "Right Hand"}
};
