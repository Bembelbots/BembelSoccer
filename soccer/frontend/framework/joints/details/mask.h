#pragma once

#include "../definitionsBody.h"


namespace joints {
namespace details {
    
// Start at 32 so nothing in this enum will collide with the 25 Joint values.
// This allows us to check for incorrect usage if any bit below is set.
enum class Mask : uint32_t {
    HeadYaw        = 32<<HEAD_YAW,
    HeadPitch      = 32<<HEAD_PITCH,
    LShoulderPitch = 32<<L_SHOULDER_PITCH,
    LShoulderRoll  = 32<<L_SHOULDER_ROLL,
    LElbowYaw      = 32<<L_ELBOW_YAW,
    LElbowRoll     = 32<<L_ELBOW_ROLL,
    LWristYaw      = 32<<L_WRIST_YAW,
    LHand          = 32<<L_HAND,
    HipYawPitch    = 32<<L_HIP_YAW_PITCH,
    LHipRoll       = 32<<L_HIP_ROLL,
    LHipPitch      = 32<<L_HIP_PITCH,
    LKneePitch     = 32<<L_KNEE_PITCH,
    LAnklePitch    = 32<<L_ANKLE_PITCH,
    LAnkleRoll     = 32<<L_ANKLE_ROLL,
    RShoulderPitch = 32<<R_SHOULDER_PITCH,
    RShoulderRoll  = 32<<R_SHOULDER_ROLL,
    RElbowYaw      = 32<<R_ELBOW_YAW,
    RElbowRoll     = 32<<R_ELBOW_ROLL,
    RWristYaw      = 32<<R_WRIST_YAW,
    RHand          = 32<<R_HAND,
    RHipRoll       = 32<<R_HIP_ROLL,
    RHipPitch      = 32<<R_HIP_PITCH,
    RKneePitch     = 32<<R_KNEE_PITCH,
    RAnklePitch    = 32<<R_ANKLE_PITCH,
    RAnkleRoll     = 32<<R_ANKLE_ROLL,

    Head  = (32<<2)  -(32<<0),
    LArm  = (32<<8)  -(32<<2),
    Hips  = (32<<9)  -(32<<8),
    LLeg  = (32<<14) -(32<<9),
    RArm  = (32<<20) -(32<<14),
    RLeg  = (32<<25) -(32<<20),
    Legs  = Hips|RLeg|LLeg,
    Arms  = LArm|RArm,
    Hands = LWristYaw|LHand|RWristYaw|RHand,
    Body  = Legs|Arms,
    All   = Body|Head,
    Old   = All & (~Hands)
};

constexpr inline Mask operator|(Mask m1, Mask m2) {
    return static_cast<Mask>(
            static_cast<uint32_t>(m1) | static_cast<uint32_t>(m2));
}

constexpr inline Mask operator&(Mask m1, Mask m2) {
    return static_cast<Mask>(
            static_cast<uint32_t>(m1) & static_cast<uint32_t>(m2));
}

constexpr inline bool any(Mask m) {
    return static_cast<bool>(m);
}

constexpr inline Mask idToMask(joint_id id) {
    return static_cast<Mask>(32 << id);
}

constexpr inline Mask intToMask(int id) {
    return idToMask(static_cast<joint_id>(id));
}


static_assert(Mask::Head == (Mask::HeadYaw | Mask::HeadPitch),
        "Bitmask Definitions are wrong");

static_assert(Mask::LArm == (Mask::LShoulderPitch 
                            | Mask::LShoulderRoll 
                            | Mask::LElbowYaw 
                            | Mask::LElbowRoll 
                            | Mask::LWristYaw 
                            | Mask::LHand),
        "Bitmask Definitions are wrong");

static_assert(Mask::RArm == (Mask::RShoulderPitch 
                      | Mask::RShoulderRoll 
                      | Mask::RElbowYaw 
                      | Mask::RElbowRoll 
                      | Mask::RWristYaw 
                      | Mask::RHand),
        "Bitmask Definitions are wrong");

static_assert(Mask::RLeg == (Mask::RHipRoll 
                            | Mask::RHipPitch 
                            | Mask::RKneePitch 
                            | Mask::RAnklePitch 
                            | Mask::RAnkleRoll),
        "Bitmask Definitions are wrong");

static_assert(Mask::LLeg == (Mask::LHipRoll 
                            | Mask::LHipPitch 
                            | Mask::LKneePitch 
                            | Mask::LAnklePitch 
                            | Mask::LAnkleRoll),
        "Bitmask Definitions are wrong");

static_assert(Mask::Hips == (Mask::HipYawPitch) ,
        "Bitmask Definitions are wrong");

} // namespace details
} // namespace joints

// vim: set ts=4 sw=4 sts=4 expandtab:
