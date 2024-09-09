#pragma once

#include <framework/blackboard/blackboard.h>

class MotionDesignBlackboard : public Blackboard {

public:
    static constexpr const char *blackboardName = "MotionDesignBlackboard";

    MotionDesignBlackboard();

    // Actuators
    MAKE_VAR(float, head_pitch);
    MAKE_VAR(float, head_yaw);
    
    MAKE_VAR(float, l_shoulder_pitch);
    MAKE_VAR(float, l_shoulder_roll);
    MAKE_VAR(float, l_elbow_yaw);
    MAKE_VAR(float, l_elbow_roll);
    MAKE_VAR(float, l_hip_yaw_pitch);
    MAKE_VAR(float, l_hip_roll);
    MAKE_VAR(float, l_hip_pitch);
    MAKE_VAR(float, l_knee_pitch);
    MAKE_VAR(float, l_ankle_pitch);
    MAKE_VAR(float, l_ankle_roll);

    MAKE_VAR(float, r_shoulder_pitch);
    MAKE_VAR(float, r_shoulder_roll);
    MAKE_VAR(float, r_elbow_yaw);
    MAKE_VAR(float, r_elbow_roll);
    MAKE_VAR(float, r_hip_roll);
    MAKE_VAR(float, r_hip_pitch);
    MAKE_VAR(float, r_knee_pitch);
    MAKE_VAR(float, r_ankle_pitch);
    MAKE_VAR(float, r_ankle_roll);

    // Stiffness
    MAKE_VAR(float, headPitchStiffness);
    MAKE_VAR(float, headYawStiffness);
    MAKE_VAR(float, lShoulderPitchStiffness);
    MAKE_VAR(float, lShoulderRollStiffness);
    MAKE_VAR(float, lElbowYawStiffness);
    MAKE_VAR(float, lElbowRollStiffness);
    MAKE_VAR(float, lWristYawStiffness);
    MAKE_VAR(float, rShoulderPitchStiffness);
    MAKE_VAR(float, rShoulderRollStiffness);
    MAKE_VAR(float, rElbowYawStiffness);
    MAKE_VAR(float, rElbowRollStiffness);
    MAKE_VAR(float, rWristYawStiffness);
    MAKE_VAR(float, lHipYawPitchStiffness);
    MAKE_VAR(float, lHipRollStiffness);
    MAKE_VAR(float, lHipPitchStiffness);
    MAKE_VAR(float, lKneePitchStiffness);
    MAKE_VAR(float, lAnklePitchStiffness);
    MAKE_VAR(float, lAnkleRollStiffness);
    MAKE_VAR(float, rHipRollStiffness);
    MAKE_VAR(float, rHipPitchStiffness);
    MAKE_VAR(float, rKneePitchStiffness);
    MAKE_VAR(float, rAnklePitchStiffness);
    MAKE_VAR(float, rAnkleRollStiffness);

    // FLAG if there will be a number of keyframes played
    MAKE_VAR(bool, isPlayAllKeyframes);
    // FLAG if the current keyframe is done interpolating
    MAKE_VAR(bool, isKeyframeInterpolationComplete);

    MAKE_VAR(bool, reReadActuators);
    MAKE_VAR(bool, jointsMoved);
    MAKE_VAR(float, stiffness);

    MAKE_VAR(int, time);

    MAKE_VAR(bool, isPlayKeyframe);
};
