#include "walking_engine.h"
#include "htwkwalk.h"
#include "framework/joints/joints.hpp"
#include "lola_names_generated.h"

#define DISABLE_ANGLE_OPERATORS
#include "../bodycontrol/blackboards/bodyblackboard.h"
#include <framework/math/directed_coord.h>

namespace detail {
        
struct WalkingEngineImpl {
    WalkingEngine *engine;

    HTWKWalk walk;
    Odometry odo;
    ArmController arms;
    AnkleBalancer ankleBalancer;
    FSR fsr;
    YPR ypr;

    static inline float position(Joint &joint) { return joint.angle; }

    static inline float stiffness(Joint &joint) { return joint.stiffness; }

    WalkingEngineImpl(WalkingEngine *engine) : engine(engine) {}

    void setRequest(float dir[3], float feet_angle = 0.0f) { walk.setRequest(dir[0], dir[1], dir[2], feet_angle); }

    bool isStanding() { return walk.isStanding(); }

    void setBodyHeightStand(float height) { walk.body_height_stand = height; }

    void setStandStiffness(float stiffness) { }

    void reset() { walk.reset(); }

    void proceed(BodyBlackboard *bb) {
        proceedOdo();
        const auto &f{bb->sensors.fsr};
        fsr.left = {f.left.fl, f.left.fr, f.left.rl, f.left.rr};
        fsr.right = {f.right.fl, f.right.fr, f.right.rl, f.right.rr};
        const auto &s{bb->sensors};
        proceedAnkelBalancer(s.imu.gyroscope);

        LegJoints joints = walk.proceed(fsr,
                bb->bodyAngles.y(), // body_pitch
                bb->bodyAngles.x(), // body_roll
                ankleBalancer,
                bb->bodyAngles.z(), // body_yaw
                &odo,
                &arms);

        updateActuators(bb->actuators, joints);

        bb->odometry = odo.getMovementVec();
    }

    void proceedOdo() {
        odo.scale_x = engine->calibration.scale_odo_x;
        odo.scale_y = engine->calibration.scale_odo_y;
    }

    void proceedAnkelBalancer(const Eigen::Vector3f &gyro) {
        ypr = YPR(gyro);
        ankleBalancer.proceed(ypr);
    }

    float getSupportFoot() {
        return 0;
    }

    void updateArms(bbipc::Actuators *actuators) {
        auto &calibration = engine->calibration;

        joints::pos::Arms j(actuators);
        j[JointNames::LShoulderPitch] = calibration.shoulder_pitch;
        j[JointNames::LShoulderRoll] = calibration.shoulder_roll;
        j[JointNames::LElbowYaw] = calibration.elbow_yaw;
        j[JointNames::LElbowRoll] = calibration.elbow_roll;

        j[JointNames::LWristYaw] = -1.5f;
        j[JointNames::LHand] = 0.0f;

        j[JointNames::RShoulderPitch] = j[JointNames::LShoulderPitch];
        j[JointNames::RShoulderRoll] = -j[JointNames::LShoulderRoll];
        j[JointNames::RElbowYaw] = -j[JointNames::LElbowYaw];
        j[JointNames::RElbowRoll] = -j[JointNames::LElbowRoll];
        j[JointNames::RWristYaw] = -j[JointNames::LWristYaw];
        j[JointNames::RHand] = 0.0f;

        j.write(actuators);
    }

    void updateLegs(bbipc::Actuators *actuators, LegJoints &joints) {
        joints::pos::Legs pos(actuators);
        joints::stiffness::Legs stiff(actuators);
        
        pos[JointNames::LHipYawPitch] = position(joints[HipYawPitch]);

        pos[JointNames::LHipRoll] = position(joints[LHipRoll]);
        pos[JointNames::LHipPitch] = position(joints[LHipPitch]);
        pos[JointNames::LKneePitch] = position(joints[LKneePitch]);
        pos[JointNames::LAnklePitch] = position(joints[LAnklePitch]);
        pos[JointNames::LAnkleRoll] = position(joints[LAnkleRoll]);

        pos[JointNames::RHipRoll] = position(joints[RHipRoll]);
        pos[JointNames::RHipPitch] = position(joints[RHipPitch]);
        pos[JointNames::RKneePitch] = position(joints[RKneePitch]);
        pos[JointNames::RAnklePitch] = position(joints[RAnklePitch]);
        pos[JointNames::RAnkleRoll] = position(joints[RAnkleRoll]);

        stiff[JointNames::LHipYawPitch] = stiffness(joints[HipYawPitch]);

        stiff[JointNames::LHipRoll] = stiffness(joints[LHipRoll]);
        stiff[JointNames::LHipPitch] = stiffness(joints[LHipPitch]);
        stiff[JointNames::LKneePitch] = stiffness(joints[LKneePitch]);
        stiff[JointNames::LAnklePitch] = stiffness(joints[LAnklePitch]);
        stiff[JointNames::LAnkleRoll] = stiffness(joints[LAnkleRoll]);

        stiff[JointNames::RHipRoll] = stiffness(joints[RHipRoll]);
        stiff[JointNames::RHipPitch] = stiffness(joints[RHipPitch]);
        stiff[JointNames::RKneePitch] = stiffness(joints[RKneePitch]);
        stiff[JointNames::RAnklePitch] = stiffness(joints[RAnklePitch]);
        stiff[JointNames::RAnkleRoll] = stiffness(joints[RAnkleRoll]);

        pos.write(actuators);
        stiff.write(actuators);
    }

    void updateActuators(bbipc::Actuators *actuators, LegJoints &legs) {
        updateArms(actuators);
        updateLegs(actuators, legs);
    }

    void reloadSettings(const WalkCalibration &calib) {
        walk.step_duration = calib.step_duration;                       // s
        walk.max_forward = calib.max_forward;                           // m/s
        walk.max_backward = calib.max_backward;                         // m/s
        walk.max_strafe = calib.max_strafe;                             // m/s
        walk.max_turn = calib.max_turn;                                 // rad/s
        walk.max_acceleration_forward = calib.max_acceleration_forward; // m/s/s
        walk.max_deceleration_forward = calib.max_deceleration_forward; // m/s/s
        walk.max_acceleration_side = calib.max_acceleration_side;       // m/s/s
        walk.max_acceleration_turn = calib.max_acceleration_turn;       // rad/s/s
        walk.min_rel_step_duration = calib.min_rel_step_duration;
        walk.max_rel_step_duration = calib.max_rel_step_duration;
        walk.step_duration_factor = calib.step_duration_factor;
        walk.body_shift_amp = calib.body_shift_amp;
        walk.body_shift_smoothing = calib.body_shift_smoothing;
        walk.body_offset_x = calib.body_offset_x;
        walk.waddle_gain = calib.waddle_gain;
        walk.stairway_gain = calib.stairway_gain;
        walk.basic_step_height = calib.basic_step_height;
        walk.forward_step_height = calib.forward_step_height;
        walk.support_recover_factor = calib.support_recover_factor;
        walk.velocity_combination_damping = calib.velocity_combination_damping;
    }
};

} // namespace detail

WalkingEngine::WalkingEngine() {
    impl = std::make_shared<detail::WalkingEngineImpl>(this);
}

void WalkingEngine::reset() {
    impl->reset();
    setBodyHeight(BodyHeight::NORMAL);
}

void WalkingEngine::proceed(BodyBlackboard *bb) {
    impl->reloadSettings(calibration);
    impl->proceed(bb);
}

void WalkingEngine::setDirection(DirectedCoord dir) {
    float wdir[3] = {dir.coord.x, dir.coord.y, dir.angle.rad()};

    jsassert(std::isnan(wdir[0]) == false);
    jsassert(std::isnan(wdir[1]) == false);
    jsassert(std::isnan(wdir[2]) == false);

    wdir[0] *= (wdir[0] > 0.0f ? calibration.max_forward : calibration.max_backward);
    wdir[1] *= calibration.max_strafe;
    wdir[2] *= calibration.max_turn;

    impl->setRequest(wdir, calibration.feet_angle);
}

void WalkingEngine::setBodyHeight(BodyHeight height) {
    switch (height) {
        case BodyHeight::HIGH:
            impl->setBodyHeightStand(bodyHeightHighStand);
            break;
        case BodyHeight::NORMAL:
            impl->setBodyHeightStand(calibration.body_height);
            break;
    }
}

void WalkingEngine::setStiffness(float stiffness) {
    impl->setStandStiffness(stiffness);
}

void WalkingEngine::setStand(bool stand) {
    if (!stand) {
        return;
    }
    float wdir[3] = {0.0f, 0.0f, 0.0f};
    impl->setRequest(wdir, 0.0f);
}

bool WalkingEngine::isStanding() {
    return impl->isStanding();
}

float WalkingEngine::getSupportFoot() {
    return impl->getSupportFoot();
}
