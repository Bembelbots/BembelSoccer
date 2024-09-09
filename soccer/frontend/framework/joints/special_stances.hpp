#pragma once

#include <representations/bembelbots/constants.h>
#include <framework/joints/joints.hpp>

static const joints::pos::All STANDUP_STANCE({
    .headYaw        = 0.0f,
    .headPitch      = CONST::initial_pitch,

    .lShoulderPitch = 1.868f,
    .lShoulderRoll  = 0.03f,
    .lElbowYaw      = -1.6f,
    .lElbowRoll     = -0.3f,
    .lWristYaw      = -1.5f,

    .hipYawPitch    = 0.0f,

    .lHipRoll       = 0.0001f,
    .lHipPitch      = 0.049f,
    .lKneePitch     = 0.f,
    .lAnklePitch    = -0.049f,
    .lAnkleRoll     = 0.0001f,

    .rHipRoll       = 0.0001f,
    .rHipPitch      = 0.049f,
    .rKneePitch     = 0.f,
    .rAnklePitch    = -0.049f,
    .rAnkleRoll     = 0.0001f,
 
    .rShoulderPitch = 1.868f,
    .rShoulderRoll  = -0.03f,
    .rElbowYaw      = 1.6f,
    .rElbowRoll     = 0.3,
    .rWristYaw      = 1.5,
   
    .lHand          = 0.f,
    .rHand          = 0.f,
});

static const joints::pos::All SIT_STANCE({
	.headYaw        = 0.f,
	.headPitch      = CONST::initial_pitch, 

	.lShoulderPitch = 0.89f,
	.lShoulderRoll  = 0.06f,
	.lElbowYaw      = 0.26f,
	.lElbowRoll     = -0.62f,
	.lWristYaw      = -1.57f,

	.hipYawPitch    = 0.f,

	.lHipRoll       = 0.f,
	.lHipPitch      = -0.87f,
	.lKneePitch     = 2.11f,
	.lAnklePitch    = -1.18f,
	.lAnkleRoll     = 0.f,

	.rHipRoll       = 0.f,
	.rHipPitch      = -0.87f,
	.rKneePitch     = 2.11f,
	.rAnklePitch    = -1.18f,
	.rAnkleRoll     = 0.f,

	.rShoulderPitch = 0.89f,
	.rShoulderRoll  = -0.06f,
	.rElbowYaw      = -0.26f,
	.rElbowRoll     = 0.62f,
	.rWristYaw      = 1.57f,

	.lHand          = 0.f,
    .rHand          = 0.f,
});


