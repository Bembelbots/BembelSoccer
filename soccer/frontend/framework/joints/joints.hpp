#pragma once

#include "details/cubic.hpp"
#include "details/interpolation.hpp"
#include "details/linear.hpp"
#include "details/operators.hpp"
#include "details/pos.hpp"
#include "details/stiffness.hpp"
#include "details/current.hpp"
#include "details/status.hpp"
#include "details/temperature.hpp"

#include <representations/flatbuffers/types/lola_names.h>

namespace joints {

using details::Cubic;
using details::each;
using details::feq;
using details::Interpolation;
using details::Joints;
using details::Linear;
using details::Mask;
using details::Pos;
using details::Stiffness;
using details::Current;
using details::Status;
using details::Temperature;

// clang-format off
namespace pos {
using HeadYaw        = Pos<Mask::HeadYaw>;
using HeadPitch      = Pos<Mask::HeadPitch>;
using LShoulderPitch = Pos<Mask::LShoulderPitch>;
using LShoulderRoll  = Pos<Mask::LShoulderRoll>;
using LElbowYaw      = Pos<Mask::LElbowYaw>;
using LElbowRoll     = Pos<Mask::LElbowRoll>;
using LWristYaw      = Pos<Mask::LWristYaw>;
using LHand          = Pos<Mask::LHand>;
using HipYawPitch    = Pos<Mask::LHipYawPitch>;
using LHipRoll       = Pos<Mask::LHipRoll>;
using LHipPitch      = Pos<Mask::LHipPitch>;
using LKneePitch     = Pos<Mask::LKneePitch>;
using LAnklePitch    = Pos<Mask::LAnklePitch>;
using LAnkleRoll     = Pos<Mask::LAnkleRoll>;
using RShoulderPitch = Pos<Mask::RShoulderPitch>;
using RShoulderRoll  = Pos<Mask::RShoulderRoll>;
using RElbowYaw      = Pos<Mask::RElbowYaw>;
using RElbowRoll     = Pos<Mask::RElbowRoll>;
using RWristYaw      = Pos<Mask::RWristYaw>;
using RHand          = Pos<Mask::RHand>;
using RHipRoll       = Pos<Mask::RHipRoll>;
using RHipPitch      = Pos<Mask::RHipPitch>;
using RKneePitch     = Pos<Mask::RKneePitch>;
using RAnklePitch    = Pos<Mask::RAnklePitch>;
using RAnkleRoll     = Pos<Mask::RAnkleRoll>;

using Head  = Pos<Mask::Head>;
using LArm  = Pos<Mask::LArm>;
using Hips  = Pos<Mask::Hips>;
using LLeg  = Pos<Mask::LLeg>;
using RArm  = Pos<Mask::RArm>;
using RLeg  = Pos<Mask::RLeg>;
using Legs  = Pos<Mask::Legs>;
using Arms  = Pos<Mask::Arms>;
using Hands = Pos<Mask::Hands>;
using Body  = Pos<Mask::Body>;
using All   = Pos<Mask::All>;
using Old   = Pos<Mask::Old>;
} // namespace pos

namespace current {
    using HeadYaw        = Current<Mask::HeadYaw>;
    using HeadPitch      = Current<Mask::HeadPitch>;
    using LShoulderPitch = Current<Mask::LShoulderPitch>;
    using LShoulderRoll  = Current<Mask::LShoulderRoll>;
    using LElbowYaw      = Current<Mask::LElbowYaw>;
    using LElbowRoll     = Current<Mask::LElbowRoll>;
    using LWristYaw      = Current<Mask::LWristYaw>;
    using LHand          = Current<Mask::LHand>;
    using HipYawPitch    = Current<Mask::LHipYawPitch>;
    using LHipRoll       = Current<Mask::LHipRoll>;
    using LHipPitch      = Current<Mask::LHipPitch>;
    using LKneePitch     = Current<Mask::LKneePitch>;
    using LAnklePitch    = Current<Mask::LAnklePitch>;
    using LAnkleRoll     = Current<Mask::LAnkleRoll>;
    using RShoulderPitch = Current<Mask::RShoulderPitch>;
    using RShoulderRoll  = Current<Mask::RShoulderRoll>;
    using RElbowYaw      = Current<Mask::RElbowYaw>;
    using RElbowRoll     = Current<Mask::RElbowRoll>;
    using RWristYaw      = Current<Mask::RWristYaw>;
    using RHand          = Current<Mask::RHand>;
    using RHipRoll       = Current<Mask::RHipRoll>;
    using RHipPitch      = Current<Mask::RHipPitch>;
    using RKneePitch     = Current<Mask::RKneePitch>;
    using RAnklePitch    = Current<Mask::RAnklePitch>;
    using RAnkleRoll     = Current<Mask::RAnkleRoll>;

    using Head  = Current<Mask::Head>;
    using LArm  = Current<Mask::LArm>;
    using Hips  = Current<Mask::Hips>;
    using LLeg  = Current<Mask::LLeg>;
    using RArm  = Current<Mask::RArm>;
    using RLeg  = Current<Mask::RLeg>;
    using Legs  = Current<Mask::Legs>;
    using Arms  = Current<Mask::Arms>;
    using Hands = Current<Mask::Hands>;
    using Body  = Current<Mask::Body>;
    using All   = Current<Mask::All>;
    using Old   = Current<Mask::Old>;
} // namespace current

namespace status {
    using HeadYaw        = Status<Mask::HeadYaw>;
    using HeadPitch      = Status<Mask::HeadPitch>;
    using LShoulderPitch = Status<Mask::LShoulderPitch>;
    using LShoulderRoll  = Status<Mask::LShoulderRoll>;
    using LElbowYaw      = Status<Mask::LElbowYaw>;
    using LElbowRoll     = Status<Mask::LElbowRoll>;
    using LWristYaw      = Status<Mask::LWristYaw>;
    using LHand          = Status<Mask::LHand>;
    using HipYawPitch    = Status<Mask::LHipYawPitch>;
    using LHipRoll       = Status<Mask::LHipRoll>;
    using LHipPitch      = Status<Mask::LHipPitch>;
    using LKneePitch     = Status<Mask::LKneePitch>;
    using LAnklePitch    = Status<Mask::LAnklePitch>;
    using LAnkleRoll     = Status<Mask::LAnkleRoll>;
    using RShoulderPitch = Status<Mask::RShoulderPitch>;
    using RShoulderRoll  = Status<Mask::RShoulderRoll>;
    using RElbowYaw      = Status<Mask::RElbowYaw>;
    using RElbowRoll     = Status<Mask::RElbowRoll>;
    using RWristYaw      = Status<Mask::RWristYaw>;
    using RHand          = Status<Mask::RHand>;
    using RHipRoll       = Status<Mask::RHipRoll>;
    using RHipPitch      = Status<Mask::RHipPitch>;
    using RKneePitch     = Status<Mask::RKneePitch>;
    using RAnklePitch    = Status<Mask::RAnklePitch>;
    using RAnkleRoll     = Status<Mask::RAnkleRoll>;

    using Head  = Status<Mask::Head>;
    using LArm  = Status<Mask::LArm>;
    using Hips  = Status<Mask::Hips>;
    using LLeg  = Status<Mask::LLeg>;
    using RArm  = Status<Mask::RArm>;
    using RLeg  = Status<Mask::RLeg>;
    using Legs  = Status<Mask::Legs>;
    using Arms  = Status<Mask::Arms>;
    using Hands = Status<Mask::Hands>;
    using Body  = Status<Mask::Body>;
    using All   = Status<Mask::All>;
    using Old   = Status<Mask::Old>;
} // namespace status


namespace temperature {
using HeadYaw        = Temperature<Mask::HeadYaw>;
using HeadPitch      = Temperature<Mask::HeadPitch>;
using LShoulderPitch = Temperature<Mask::LShoulderPitch>;
using LShoulderRoll  = Temperature<Mask::LShoulderRoll>;
using LElbowYaw      = Temperature<Mask::LElbowYaw>;
using LElbowRoll     = Temperature<Mask::LElbowRoll>;
using LWristYaw      = Temperature<Mask::LWristYaw>;
using LHand          = Temperature<Mask::LHand>;
using HipYawPitch    = Temperature<Mask::LHipYawPitch>;
using LHipRoll       = Temperature<Mask::LHipRoll>;
using LHipPitch      = Temperature<Mask::LHipPitch>;
using LKneePitch     = Temperature<Mask::LKneePitch>;
using LAnklePitch    = Temperature<Mask::LAnklePitch>;
using LAnkleRoll     = Temperature<Mask::LAnkleRoll>;
using RShoulderPitch = Temperature<Mask::RShoulderPitch>;
using RShoulderRoll  = Temperature<Mask::RShoulderRoll>;
using RElbowYaw      = Temperature<Mask::RElbowYaw>;
using RElbowRoll     = Temperature<Mask::RElbowRoll>;
using RWristYaw      = Temperature<Mask::RWristYaw>;
using RHand          = Temperature<Mask::RHand>;
using RHipRoll       = Temperature<Mask::RHipRoll>;
using RHipPitch      = Temperature<Mask::RHipPitch>;
using RKneePitch     = Temperature<Mask::RKneePitch>;
using RAnklePitch    = Temperature<Mask::RAnklePitch>;
using RAnkleRoll     = Temperature<Mask::RAnkleRoll>;

using Head  = Temperature<Mask::Head>;
using LArm  = Temperature<Mask::LArm>;
using Hips  = Temperature<Mask::Hips>;
using LLeg  = Temperature<Mask::LLeg>;
using RArm  = Temperature<Mask::RArm>;
using RLeg  = Temperature<Mask::RLeg>;
using Legs  = Temperature<Mask::Legs>;
using Arms  = Temperature<Mask::Arms>;
using Hands = Temperature<Mask::Hands>;
using Body  = Temperature<Mask::Body>;
using All   = Temperature<Mask::All>;
using Old   = Temperature<Mask::Old>;
} // namespace temeperature

namespace stiffness {
using HeadYaw        = Stiffness<Mask::HeadYaw>;
using HeadPitch      = Stiffness<Mask::HeadPitch>;
using LShoulderPitch = Stiffness<Mask::LShoulderPitch>;
using LShoulderRoll  = Stiffness<Mask::LShoulderRoll>;
using LElbowYaw      = Stiffness<Mask::LElbowYaw>;
using LElbowRoll     = Stiffness<Mask::LElbowRoll>;
using LWristYaw      = Stiffness<Mask::LWristYaw>;
using LHand          = Stiffness<Mask::LHand>;
using HipYawPitch    = Stiffness<Mask::LHipYawPitch>;
using LHipRoll       = Stiffness<Mask::LHipRoll>;
using LHipPitch      = Stiffness<Mask::LHipPitch>;
using LKneePitch     = Stiffness<Mask::LKneePitch>;
using LAnklePitch    = Stiffness<Mask::LAnklePitch>;
using LAnkleRoll     = Stiffness<Mask::LAnkleRoll>;
using RShoulderPitch = Stiffness<Mask::RShoulderPitch>;
using RShoulderRoll  = Stiffness<Mask::RShoulderRoll>;
using RElbowYaw      = Stiffness<Mask::RElbowYaw>;
using RElbowRoll     = Stiffness<Mask::RElbowRoll>;
using RWristYaw      = Stiffness<Mask::RWristYaw>;
using RHand          = Stiffness<Mask::RHand>;
using RHipRoll       = Stiffness<Mask::RHipRoll>;
using RHipPitch      = Stiffness<Mask::RHipPitch>;
using RKneePitch     = Stiffness<Mask::RKneePitch>;
using RAnklePitch    = Stiffness<Mask::RAnklePitch>;
using RAnkleRoll     = Stiffness<Mask::RAnkleRoll>;

using Head  = Stiffness<Mask::Head>;
using LArm  = Stiffness<Mask::LArm>;
using Hips  = Stiffness<Mask::Hips>;
using LLeg  = Stiffness<Mask::LLeg>;
using RArm  = Stiffness<Mask::RArm>;
using RLeg  = Stiffness<Mask::RLeg>;
using Legs  = Stiffness<Mask::Legs>;
using Arms  = Stiffness<Mask::Arms>;
using Hands = Stiffness<Mask::Hands>;
using Body  = Stiffness<Mask::Body>;
using All   = Stiffness<Mask::All>;
using Old   = Stiffness<Mask::Old>;
} // namespace stiffness
// clang-format on

} // namespace joints
