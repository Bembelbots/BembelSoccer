#pragma once

#include <representations/flatbuffers/types/lola_names.h>

namespace joints {
namespace details {

// Start at 32 so nothing in this enum will collide with the 25 Joint values.
// This allows us to check for incorrect usage if any bit below is set.
enum class Mask : uint32_t {
    HeadYaw        = 32<<JointNames::HeadYaw,
    HeadPitch      = 32<<JointNames::HeadPitch,
    LShoulderPitch = 32<<JointNames::LShoulderPitch,
    LShoulderRoll  = 32<<JointNames::LShoulderRoll,
    LElbowYaw      = 32<<JointNames::LElbowYaw,
    LElbowRoll     = 32<<JointNames::LElbowRoll,
    LWristYaw      = 32<<JointNames::LWristYaw,
    LHipYawPitch   = 32<<JointNames::LHipYawPitch,
    LHipRoll       = 32<<JointNames::LHipRoll,
    LHipPitch      = 32<<JointNames::LHipPitch,
    LKneePitch     = 32<<JointNames::LKneePitch,
    LAnklePitch    = 32<<JointNames::LAnklePitch,
    LAnkleRoll     = 32<<JointNames::LAnkleRoll,
    RHipRoll       = 32<<JointNames::RHipRoll,
    RHipPitch      = 32<<JointNames::RHipPitch,
    RKneePitch     = 32<<JointNames::RKneePitch,
    RAnklePitch    = 32<<JointNames::RAnklePitch,
    RAnkleRoll     = 32<<JointNames::RAnkleRoll,
    RShoulderPitch = 32<<JointNames::RShoulderPitch,
    RShoulderRoll  = 32<<JointNames::RShoulderRoll,
    RElbowYaw      = 32<<JointNames::RElbowYaw,
    RElbowRoll     = 32<<JointNames::RElbowRoll,
    RWristYaw      = 32<<JointNames::RWristYaw,
    LHand          = 32<<JointNames::LHand,
    RHand          = 32<<JointNames::RHand,

    Head  = HeadYaw | HeadPitch,
    LArm  = LShoulderPitch | LShoulderRoll | LElbowYaw | LElbowRoll | LWristYaw | LHand,
    Hips  = LHipYawPitch,
    LLeg  = LHipRoll | LHipPitch | LKneePitch | LAnklePitch | LAnkleRoll,
    RArm  = RShoulderPitch | RShoulderRoll | RElbowYaw | RElbowRoll | RWristYaw | RHand,
    RLeg  = RHipRoll | RHipPitch | RKneePitch | RAnklePitch | RAnkleRoll,
    Legs  = Hips | LLeg | RLeg,
    Arms  = LArm | RArm,
    Hands = LWristYaw | LHand | RWristYaw | RHand,
    Body  = Legs | Arms,
    All   = Body | Head,
    Old   = All & ~Hands,

    None  = 0
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

constexpr inline Mask idToMask(JointNames id) {
    return static_cast<Mask>(32 << id);
}

constexpr inline Mask intToMask(int id) {
    return idToMask(static_cast<JointNames>(id));
}

constexpr inline bool operator&(Mask m, JointNames j) {
    return any(m & idToMask(j));
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

static_assert(Mask::Hips == (Mask::LHipYawPitch),
        "Bitmask Definitions are wrong");

} // namespace details
} // namespace joints

// vim: set ts=4 sw=4 sts=4 expandtab:
