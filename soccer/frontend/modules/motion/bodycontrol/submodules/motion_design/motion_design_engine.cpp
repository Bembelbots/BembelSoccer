#include "motion_design_engine.h"
#include "framework/joints/details/operators.hpp"
#include "framework/joints/details/stiffness.hpp"
#include <iomanip>
#include <lola_names_generated.h>

#include <framework/joints/joints.hpp>

#include <bodycontrol/blackboards/bodyblackboard.h>
#include <bodycontrol/blackboards/motion_design_blackboard.h>

using joints::Mask;

void MotionDesignEngine::connect(rt::Linker &link) {
    link(mdbb);
}

SubModuleReturnValue MotionDesignEngine::step(BodyBlackboard *bb) {
    auto lock = mdbb->scopedLock();
    this->bb = bb;

    const auto &now{bb->timestamp_ms};
    const auto &touch{bb->sensors.touch};
    bool headTouch{touch.head.middle > 0};
    bool lHandTouch{touch.head.front > 0};
    bool rHandTouch{touch.head.rear > 0};
    bool lFootTouch{touch.feet.left.bumper_left || touch.feet.left.bumper_right};
    bool rFootTouch{touch.feet.right.bumper_left || touch.feet.right.bumper_right};

    auto stiff{set_stiffness()};
    stiff.write(bb->actuators);

    auto &led{bb->actuators->led};
    led.eyes.left.fill(1);
    led.eyes.right.fill(1);
    led.skull.fill(0);

    if (lHandTouch || rHandTouch || lFootTouch || rFootTouch || headTouch) {
        if (lHandTouch)
            updateMask = joints::Mask::LArm;
        else if (rHandTouch)
            updateMask = joints::Mask::RArm;
        else if (lFootTouch)
            updateMask = joints::Mask::LLeg | Mask::LHipYawPitch;
        else if (rFootTouch)
            updateMask = joints::Mask::RLeg | Mask::LHipYawPitch;
        else if (headTouch)
            updateMask = joints::Mask::Head;

        mdbb->reReadActuators = true;
        unstiff_chain(updateMask);
        return MOTION_UNSTABLE;
    }

    if (mdbb->reReadActuators) {
        write_to_blackboard(updateMask);
        mdbb->reReadActuators = false;
        updateMask = joints::Mask::All;
        return MOTION_UNSTABLE;
    }

    bool done{motion.done(now)};
    mdbb->isKeyframeInterpolationComplete = done;

    if (mdbb->isPlayKeyframe) {
        mdbb->isPlayKeyframe = false;
        set_motion(now, mdbb->time);
    } else if (!mdbb->isPlayAllKeyframes && done) {
        set_motion(now, 1000);
    }

    auto pos{motion.get(now)};
    pos.write(bb->actuators);

    return MOTION_UNSTABLE;
}

void MotionDesignEngine::unstiff_chain(const joints::Mask &m) {
    jsassert(bb);

    joints::pos::All actPos{bb->actuators}, curPos{bb->sensors};
    joints::stiffness::All stiff{bb->actuators};

    joints::each<Mask::All>([&](JointNames i) {
        if (m & i) {
            stiff[i] = 0;
            actPos[i] = curPos[i];
        }
    });
    stiff.write(bb->actuators);
    actPos.write(bb->actuators);

    write_to_blackboard(m);

    auto &ledSkull{bb->actuators->led.skull};
    if (m & JointNames::LShoulderPitch) {
        for (size_t i{0}; i < (ledSkull.size() / 2); ++i)
            ledSkull[i] = 1;

    } else if (m & JointNames::RShoulderPitch) {
        for (size_t i{ledSkull.size() / 2}; i < ledSkull.size(); ++i)
            ledSkull[i] = 1;
    }
}

joints::stiffness::All MotionDesignEngine::set_stiffness() {
    joints::stiffness::All stiff;
    stiff.fill(mdbb->stiffness);

    stiff[JointNames::HeadPitch] = mdbb->headPitchStiffness;
    stiff[JointNames::HeadYaw] = mdbb->headYawStiffness;

    stiff[JointNames::LShoulderPitch] = mdbb->lShoulderPitchStiffness;
    stiff[JointNames::LShoulderRoll] = mdbb->lShoulderRollStiffness;
    stiff[JointNames::LElbowYaw] = mdbb->lElbowYawStiffness;
    stiff[JointNames::LElbowRoll] = mdbb->lElbowRollStiffness;

    stiff[JointNames::LHipYawPitch] = mdbb->lHipYawPitchStiffness;
    stiff[JointNames::LHipRoll] = mdbb->lHipRollStiffness;
    stiff[JointNames::LHipPitch] = mdbb->lHipPitchStiffness;
    stiff[JointNames::LKneePitch] = mdbb->lKneePitchStiffness;
    stiff[JointNames::LAnklePitch] = mdbb->lAnklePitchStiffness;
    stiff[JointNames::LAnkleRoll] = mdbb->lAnkleRollStiffness;

    stiff[JointNames::RHipRoll] = mdbb->rHipRollStiffness;
    stiff[JointNames::RHipPitch] = mdbb->rHipPitchStiffness;
    stiff[JointNames::RKneePitch] = mdbb->rKneePitchStiffness;
    stiff[JointNames::RAnklePitch] = mdbb->rAnklePitchStiffness;
    stiff[JointNames::RAnkleRoll] = mdbb->rAnkleRollStiffness;
    stiff[JointNames::RShoulderPitch] = mdbb->rShoulderPitchStiffness;
    stiff[JointNames::RShoulderRoll] = mdbb->rShoulderRollStiffness;
    stiff[JointNames::RElbowYaw] = mdbb->rElbowYawStiffness;
    stiff[JointNames::RElbowRoll] = mdbb->rElbowRollStiffness;

    return stiff;
}

void MotionDesignEngine::write_to_blackboard(const joints::Mask &m) {
    joints::pos::All currentPos{bb->sensors};
    auto bbPos{read_from_blackboard()};
    bbPos.each([&](JointNames i) {
        if (m & i)
            bbPos[i] = currentPos[i];
    });

    mdbb->head_pitch = bbPos[JointNames::HeadPitch];
    mdbb->head_yaw = bbPos[JointNames::HeadYaw];

    mdbb->l_shoulder_pitch = bbPos[JointNames::LShoulderPitch];
    mdbb->l_shoulder_roll = bbPos[JointNames::LShoulderRoll];
    mdbb->l_elbow_yaw = bbPos[JointNames::LElbowYaw];
    mdbb->l_elbow_roll = bbPos[JointNames::LElbowRoll];

    mdbb->r_shoulder_pitch = bbPos[JointNames::RShoulderPitch];
    mdbb->r_shoulder_roll = bbPos[JointNames::RShoulderRoll];
    mdbb->r_elbow_yaw = bbPos[JointNames::RElbowYaw];
    mdbb->r_elbow_roll = bbPos[JointNames::RElbowRoll];

    mdbb->l_hip_yaw_pitch = bbPos[JointNames::LHipYawPitch];

    mdbb->l_hip_roll = bbPos[JointNames::LHipRoll];
    mdbb->l_hip_pitch = bbPos[JointNames::LHipPitch];
    mdbb->l_knee_pitch = bbPos[JointNames::LKneePitch];
    mdbb->l_ankle_pitch = bbPos[JointNames::LAnklePitch];
    mdbb->l_ankle_roll = bbPos[JointNames::LAnkleRoll];

    mdbb->r_hip_roll = bbPos[JointNames::RHipRoll];
    mdbb->r_hip_pitch = bbPos[JointNames::RHipPitch];
    mdbb->r_knee_pitch = bbPos[JointNames::RKneePitch];
    mdbb->r_ankle_pitch = bbPos[JointNames::RAnklePitch];
    mdbb->r_ankle_roll = bbPos[JointNames::RAnkleRoll];

    mdbb->jointsMoved = true;
}

void MotionDesignEngine::set_motion(TimestampMs start, TimestampMs duration) {
    // skip if target pose is already at blackboard pose
    joints::pos::All currentPos{bb->actuators};
    const joints::pos::All bbPos = read_from_blackboard();
    const joints::pos::All target = motion.get(INT_MAX);
    if (joints::details::feq(bbPos, target, 1e-3))
        return;

    motion.setStart(start);
    motion.setDuration(duration);
    motion.setFrom(joints::pos::All(bb->sensors));
    motion.setTo(bbPos);
}

joints::pos::All MotionDesignEngine::read_from_blackboard() {
    joints::pos::All pos{bb->actuators};

    pos[JointNames::HeadPitch] = mdbb->head_pitch;
    pos[JointNames::HeadYaw] = mdbb->head_yaw;

    pos[JointNames::LShoulderPitch] = mdbb->l_shoulder_pitch;
    pos[JointNames::LShoulderRoll] = mdbb->l_shoulder_roll;
    pos[JointNames::LElbowYaw] = mdbb->l_elbow_yaw;
    pos[JointNames::LElbowRoll] = mdbb->l_elbow_roll;

    pos[JointNames::RShoulderPitch] = mdbb->r_shoulder_pitch;
    pos[JointNames::RShoulderRoll] = mdbb->r_shoulder_roll;
    pos[JointNames::RElbowYaw] = mdbb->r_elbow_yaw;
    pos[JointNames::RElbowRoll] = mdbb->r_elbow_roll;

    pos[JointNames::LHipYawPitch] = mdbb->l_hip_yaw_pitch;

    pos[JointNames::LHipRoll] = mdbb->l_hip_roll;
    pos[JointNames::LHipPitch] = mdbb->l_hip_pitch;
    pos[JointNames::LKneePitch] = mdbb->l_knee_pitch;
    pos[JointNames::LAnklePitch] = mdbb->l_ankle_pitch;
    pos[JointNames::LAnkleRoll] = mdbb->l_ankle_roll;

    pos[JointNames::RHipRoll] = mdbb->r_hip_roll;
    pos[JointNames::RHipPitch] = mdbb->r_hip_pitch;
    pos[JointNames::RKneePitch] = mdbb->r_knee_pitch;
    pos[JointNames::RAnklePitch] = mdbb->r_ankle_pitch;
    pos[JointNames::RAnkleRoll] = mdbb->r_ankle_roll;

    return pos;
}
