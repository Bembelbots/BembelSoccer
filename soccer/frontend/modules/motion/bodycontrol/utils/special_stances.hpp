#pragma once

#include <representations/bembelbots/constants.h>
#include <framework/joints/joints.hpp>


static const joints::pos::All STANDUP_STANCE({
    .headYaw        = 0.0f,  // HEAD_YAW
    .headPitch      = CONST::initial_pitch, // HEAD_PITCH

    .lShoulderPitch = 1.868f, // LEFT_SHOULDER_PITCH
    .lShoulderRoll  = 0.03f,  // LEFT_SHOULDER_ROLL
    .lElbowYaw      = -1.6f,  // LEFT_ELBOW_YAW
    .lElbowRoll     = -0.3f,// LEFT_ELBOW_ROLL
    .lWristYaw      = -1.5f,   // LEFT_WRIST_YAW
    .lHand          = 0.0f,   // LEFT_HAND

    .hipYawPitch    = 0.0f,   // HIP_YAW_PITCH

    .lHipRoll       = 0.0001f,   // LEFT_HIP_ROLL
    .lHipPitch      = 0.049f,   // LEFT_HIP_PITCH
    .lKneePitch     = 0.f,   // LEFT_KNEE_PITCH
    .lAnklePitch    = -0.049f, // LEFT_ANKEL_PITCH
    .lAnkleRoll     = 0.0001f,   // LEFT_ANKEL_ROLL

    .rShoulderPitch = 1.868f, // RIGHT_SHOULDER_PITCH
    .rShoulderRoll  = -0.03f, // RIGHT_SHOULDER_ROLL
    .rElbowYaw      = 1.6f,   // RIGHT_ELBOW_YAW
    .rElbowRoll     = 0.3, // RIGHT_ELBOW_ROLL
    .rWristYaw      = 1.5,    // RIGHT_WRIST_YAW
    .rHand          = 0.0f,   // RIGHT_HAND

    .rHipRoll       = 0.0001f,   // RIGHT_HIP_ROLL
    .rHipPitch      = 0.049f,   // RIGHT_HIP_PITCH
    .rKneePitch     = 0.f,   // RIGHT_KNEE_PITCH
    .rAnklePitch    = -0.049f, // RIGHT_ANKEL_PITCH
    .rAnkleRoll     = 0.0001f    // RIGHT_ANKEL_ROLL
});

static const joints::pos::All SIT_STANCE({
	.headYaw = 0.f,
	.headPitch      = CONST::initial_pitch, // HEAD_PITCH

	.lShoulderPitch = 0.89f,
	.lShoulderRoll = 0.06f,
	.lElbowYaw = 0.26f,
	.lElbowRoll = -0.62f,
	.lWristYaw = -1.57f,
	.lHand = 0.f,

	.hipYawPitch = 0.f,

	.lHipRoll = 0.f,
	.lHipPitch = -0.87f,
	.lKneePitch = 2.16f,
	.lAnklePitch = -1.18f,
	.lAnkleRoll = 0.f,

	.rShoulderPitch = 0.89f,
	.rShoulderRoll = -0.06f,
	.rElbowYaw = -0.26f,
	.rElbowRoll = 0.62f,
	.rWristYaw = 1.57f,
	.rHand = 0.f,

	.rHipRoll = 0.f,
	.rHipPitch = -0.87f,
	.rKneePitch = 2.16f,
	.rAnklePitch = -1.18f,
	.rAnkleRoll = 0.f,
});
