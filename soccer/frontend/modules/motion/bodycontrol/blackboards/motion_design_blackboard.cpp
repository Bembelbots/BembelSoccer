#include "motion_design_blackboard.h"
#include <representations/bembelbots/constants.h>

MotionDesignBlackboard::MotionDesignBlackboard() : Blackboard(blackboardName) {

    // Actuators
    INIT_VAR_RW(head_pitch, CONST::initial_pitch, "");
    INIT_VAR_RW(head_yaw, CONST::initial_yaw, "");
    INIT_VAR_RW(l_shoulder_pitch, 1.64f, "");
    INIT_VAR_RW(l_shoulder_roll, 0.0f, "");
    INIT_VAR_RW(l_elbow_yaw, 0.0f, "");
    INIT_VAR_RW(l_elbow_roll, 0.0f, "");
    INIT_VAR_RW(l_hip_yaw_pitch, 0.0f, "");
    INIT_VAR_RW(l_hip_roll, 0.0f, "");
    INIT_VAR_RW(l_hip_pitch, 0.0f, "");
    INIT_VAR_RW(l_knee_pitch, 0.0f, "");
    INIT_VAR_RW(l_ankle_pitch, 0.0f, "");
    INIT_VAR_RW(l_ankle_roll, 0.0f, "");
    INIT_VAR_RW(r_shoulder_pitch, 1.648f, "");
    INIT_VAR_RW(r_shoulder_roll, 0.03f, "");
    INIT_VAR_RW(r_elbow_yaw, -1.6f, "");
    INIT_VAR_RW(r_elbow_roll, -0.3f, "");
    INIT_VAR_RW(r_hip_roll, 0.0f, "");
    INIT_VAR_RW(r_hip_pitch, 0.0f, "");
    INIT_VAR_RW(r_knee_pitch, 0.0f, "");
    INIT_VAR_RW(r_ankle_pitch, 0.0f, "");
    INIT_VAR_RW(r_ankle_roll, 0.0f, "");

    // Stiffness
    float initStiffness = 0.7f;
    INIT_VAR_RW(headPitchStiffness, initStiffness, "");
    INIT_VAR_RW(headYawStiffness, initStiffness, "");
    INIT_VAR_RW(lShoulderPitchStiffness, initStiffness, "");
    INIT_VAR_RW(lShoulderRollStiffness, initStiffness, "");
    INIT_VAR_RW(lElbowYawStiffness, initStiffness, "");
    INIT_VAR_RW(lElbowRollStiffness, initStiffness, "");
    INIT_VAR_RW(lWristYawStiffness, initStiffness, "");
    INIT_VAR_RW(rShoulderPitchStiffness, initStiffness, "");
    INIT_VAR_RW(rShoulderRollStiffness, initStiffness, "");
    INIT_VAR_RW(rElbowYawStiffness, initStiffness, "");
    INIT_VAR_RW(rElbowRollStiffness, initStiffness, "");
    INIT_VAR_RW(rWristYawStiffness, initStiffness, "");
    INIT_VAR_RW(lHipYawPitchStiffness, initStiffness, "");
    INIT_VAR_RW(lHipRollStiffness, initStiffness, "");
    INIT_VAR_RW(lHipPitchStiffness, initStiffness, "");
    INIT_VAR_RW(lKneePitchStiffness, initStiffness, "");
    INIT_VAR_RW(lAnklePitchStiffness, initStiffness, "");
    INIT_VAR_RW(lAnkleRollStiffness, initStiffness, "");
    INIT_VAR_RW(rKneePitchStiffness, initStiffness, "");
    INIT_VAR_RW(rHipRollStiffness, initStiffness, "");
    INIT_VAR_RW(rHipPitchStiffness, initStiffness, "");
    INIT_VAR_RW(rAnklePitchStiffness, initStiffness, "");
    INIT_VAR_RW(rAnkleRollStiffness, initStiffness, "");

    INIT_VAR_RW(isPlayAllKeyframes, false, "");
    INIT_VAR_RW(isKeyframeInterpolationComplete, false, "");

    INIT_VAR_RW(reReadActuators, true, "");
    INIT_VAR_RW(jointsMoved, false, "");
    INIT_VAR_RW(stiffness, initStiffness, "");

    INIT_VAR_RW(time, 1000, "");

    INIT_VAR_RW(isPlayKeyframe, true, "");
}
