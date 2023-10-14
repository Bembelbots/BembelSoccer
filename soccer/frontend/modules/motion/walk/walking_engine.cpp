#include "walking_engine.h"
#include "htwkwalk.h"

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
    
    static inline float position(Joint &joint) {
        return joint.angle;
    }
    
    static inline float stiffness(Joint &joint) {
        return joint.stiffness;
    }

    WalkingEngineImpl(WalkingEngine *engine)
        : engine(engine) {

    }

    void setRequest(float dir[3], float feet_angle = 0.0f) {
        walk.setRequest(dir[0], dir[1], dir[2], feet_angle);
    }

    bool isStanding() {
        return walk.isStanding();
    }

    void setBodyHeightStand(float height) {
        walk.body_height_stand = height;
    }

    void setStandStiffness(float stiffness) {
        //walk.stiffness_stand = stiffness;
    }

    void reset() {
        walk.reset();
    }

    void proceed(BodyBlackboard *bb) {
        proceedOdo();
        proceedFsr(bb->sensors);
        proceedAnkelBalancer(bb->gyro);

        LegJoints joints = walk.proceed(
                fsr,
                bb->bodyAngles(1), // body_pitch
                bb->bodyAngles(0), // body_roll
                ankleBalancer,
                bb->bodyAngles(2), // body_yaw
                &odo,
                &arms
        );

        updateActuators(bb->actuators, joints); 

        bb->odometry = odo.getMovementVec();
    }

    void proceedFsr(const joints::Sensors &sensors) {
        fsr.left = Foot {
            .fl = sensors[lFSRFrontLeftSensor],
            .fr = sensors[lFSRFrontRightSensor],
            .rl = sensors[lFSRRearLeftSensor],
            .rr = sensors[lFSRRearRightSensor]
        };
        
        fsr.right = Foot {
            .fl = sensors[rFSRFrontLeftSensor],
            .fr = sensors[rFSRFrontRightSensor],
            .rl = sensors[rFSRRearLeftSensor],
            .rr = sensors[rFSRRearRightSensor]
        };
    }

    void proceedOdo() {
        odo.scale_x = engine->calibration.scale_odo_x;
        odo.scale_y = engine->calibration.scale_odo_y;
    }

    void proceedAnkelBalancer(Eigen::Vector3f &gyro) {
        ypr = YPR(gyro(2), gyro(1), gyro(0));
        ankleBalancer.proceed(ypr);
    }

    void updateArms(joints::Actuators &actuators) {
        auto &calibration = engine->calibration;

        actuators[lShoulderPitchPositionActuator]   = calibration.shoulder_pitch;
        actuators[lShoulderRollPositionActuator]    = calibration.shoulder_roll;
        actuators[lElbowYawPositionActuator]        = calibration.elbow_yaw;
        actuators[lElbowRollPositionActuator]       = calibration.elbow_roll;

        actuators[lWristYawPositionActuator]        = -1.5f;
        actuators[lHandPositionActuator]            = 0.0f;

        actuators[rShoulderPitchPositionActuator]   =   actuators[lShoulderPitchPositionActuator];
        actuators[rShoulderRollPositionActuator]    = - actuators[lShoulderRollPositionActuator];
        actuators[rElbowYawPositionActuator]        = - actuators[lElbowYawPositionActuator];
        actuators[rElbowRollPositionActuator]       = - actuators[lElbowRollPositionActuator];
        actuators[rWristYawPositionActuator]        = - actuators[lWristYawPositionActuator];
        actuators[rHandPositionActuator]            = 0.0f;
    }

    void updateLegs(joints::Actuators &actuators, LegJoints &joints) {
        actuators[lHipYawPitchPositionActuator]     = position(joints[HipYawPitch]);

        actuators[lHipRollPositionActuator]         = position(joints[LHipRoll]);
        actuators[lHipPitchPositionActuator]        = position(joints[LHipPitch]);
        actuators[lKneePitchPositionActuator]       = position(joints[LKneePitch]);
        actuators[lAnklePitchPositionActuator]      = position(joints[LAnklePitch]);
        actuators[lAnkleRollPositionActuator]       = position(joints[LAnkleRoll]);

        actuators[rHipRollPositionActuator]         = position(joints[RHipRoll]);
        actuators[rHipPitchPositionActuator]        = position(joints[RHipPitch]);
        actuators[rKneePitchPositionActuator]       = position(joints[RKneePitch]);
        actuators[rAnklePitchPositionActuator]      = position(joints[RAnklePitch]);
        actuators[rAnkleRollPositionActuator]       = position(joints[RAnkleRoll]);
        
        actuators[lHipYawPitchHardnessActuator]     = stiffness(joints[HipYawPitch]);

        actuators[lHipRollHardnessActuator]         = stiffness(joints[LHipRoll]);
        actuators[lHipPitchHardnessActuator]        = stiffness(joints[LHipPitch]);
        actuators[lKneePitchHardnessActuator]       = stiffness(joints[LKneePitch]);
        actuators[lAnklePitchHardnessActuator]      = stiffness(joints[LAnklePitch]);
        actuators[lAnkleRollHardnessActuator]       = stiffness(joints[LAnkleRoll]);

        actuators[rHipRollHardnessActuator]         = stiffness(joints[RHipRoll]);
        actuators[rHipPitchHardnessActuator]        = stiffness(joints[RHipPitch]);
        actuators[rKneePitchHardnessActuator]       = stiffness(joints[RKneePitch]);
        actuators[rAnklePitchHardnessActuator]      = stiffness(joints[RAnklePitch]);
        actuators[rAnkleRollHardnessActuator]       = stiffness(joints[RAnkleRoll]);
    }
 
    void updateActuators(joints::Actuators &actuators, LegJoints &legs) {
        updateArms(actuators);
        updateLegs(actuators, legs);
    }      
        
    void reloadSettings(const WalkCalibration &calib) {
        walk.step_duration                  = calib.step_duration;              // s
        walk.max_forward                    = calib.max_forward;                // m/s
        walk.max_backward                   = calib.max_backward;               // m/s
        walk.max_strafe                     = calib.max_strafe;                 // m/s
        walk.max_turn                       = calib.max_turn;                   // rad/s
        walk.max_acceleration_forward       = calib.max_acceleration_forward;   // m/s/s
        walk.max_deceleration_forward       = calib.max_deceleration_forward;   // m/s/s
        walk.max_acceleration_side          = calib.max_acceleration_side;      // m/s/s
        walk.max_acceleration_turn          = calib.max_acceleration_turn;      // rad/s/s
        walk.min_rel_step_duration          = calib.min_rel_step_duration;
        walk.max_rel_step_duration          = calib.max_rel_step_duration;
        walk.step_duration_factor           = calib.step_duration_factor;
        walk.body_shift_amp                 = calib.body_shift_amp;
        walk.body_shift_smoothing           = calib.body_shift_smoothing;
        walk.body_offset_x                  = calib.body_offset_x;
        walk.waddle_gain                    = calib.waddle_gain;
        walk.stairway_gain                  = calib.stairway_gain;
        walk.basic_step_height              = calib.basic_step_height;
        walk.forward_step_height            = calib.forward_step_height;
        walk.support_recover_factor         = calib.support_recover_factor;
        walk.velocity_combination_damping   = calib.velocity_combination_damping;
    } 
 
}; 
 
} 
 
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
    if(!stand) {
        return;
    }
    float wdir[3] = { 0.0f, 0.0f, 0.0f };
    impl->setRequest(wdir, 0.0f);
}

bool WalkingEngine::isStanding() {
    return impl->isStanding();
}
