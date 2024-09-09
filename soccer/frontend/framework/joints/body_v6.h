#pragma once

#include <unordered_map>
#include <representations/flatbuffers/types/lola_names.h>

namespace joints {
struct Limits {
    float min{0.f}, max{0.f};
};

static const std::unordered_map<JointNames, Limits> CONSTRAINTS{
        {JointNames::HeadYaw, {-2.0857f, 2.0857f}},
        {JointNames::HeadPitch, {-0.6720f, 0.5149f}},
        {JointNames::LShoulderPitch, {-2.0857f, 2.0857f}},
        {JointNames::LShoulderRoll, {-0.3142f, 1.3265f}},
        {JointNames::LElbowYaw, {-2.0857f, 2.0857f}},
        {JointNames::LElbowRoll, {-1.5446f, -0.0349f}},
        {JointNames::LWristYaw, {-1.8238f, 1.8238f}},
        {JointNames::LHipYawPitch, {-1.145303f, 0.740810f}},
        {JointNames::LHipRoll, {-0.379472f, 0.790477f}},
        {JointNames::LHipPitch, {-1.535889f, 0.484090f}},
        {JointNames::LKneePitch, {-0.092346f, 2.112528f}},
        {JointNames::LAnklePitch, {-1.189516f, 0.922747f}},
        {JointNames::LAnkleRoll, {-0.397880f, 0.769001f}},
        {JointNames::RHipRoll, {-0.790477f, 0.379472f}},
        {JointNames::RHipPitch, {-1.535889f, 0.484090f}},
        {JointNames::RKneePitch, {-0.103083f, 2.120198f}},
        {JointNames::RAnklePitch, {-1.186448f, 0.932056f}},
        {JointNames::RAnkleRoll, {-0.768992f, 0.397935f}},
        {JointNames::RShoulderPitch, {-2.0857f, 2.0857f}},
        {JointNames::RShoulderRoll, {-1.3265f, 0.3142f}},
        {JointNames::RElbowYaw, {-2.0857f, 2.0857f}},
        {JointNames::RElbowRoll, {0.0349f, 1.5446f}},
        {JointNames::RWristYaw, {-1.8238f, 1.8238f}},
        {JointNames::LHand, {0.f, 1.f}},
        {JointNames::RHand, {0.f, 1.f}}
};
} // namespace joints
