#pragma once

#include <bodycontrol/internals/submodule.h>
#include <bodycontrol/blackboards/motion_design_blackboard.h>
#include <bodycontrol/submodules/motion_design/motion_design_engine.h>

#include <bodycontrol/utils/special_stances.hpp>
#include <framework/joints/joints.hpp>

#include <cstring>
#include <cstdarg>
#include <vector>

#include <bodycontrol/utils/state_machine.h>
#include "../../../kinematics/foot.h"
#include "../../../kinematics/step.h"
#include "../../../walk/htwk/ankle_balancer.h"
#include <bodycontrol/blackboards/bodyblackboard.h>

// Rewriting the BembelWalk is better - but we lack time.
class MotionWrapper : public SubModule {
public:
    //using Mode = WalkingEngine::Mode;

    MotionWrapper() = default;

    ~MotionWrapper() {
    }

    void connect(rt::Linker &link) override { link(calib); }

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        auto lock = calib->scopedLock();
        if(!calib->kick_active){
            std::cout << "deactivating kick" << std::endl;
            //calib->kick_active = true;
            //kick.reset();
            return DEACTIVATE_ME;
        }

        foot_s leftFoot(
                bb->sensors[lFSRFrontLeftSensor],
                bb->sensors[lFSRFrontRightSensor],
                bb->sensors[lFSRRearLeftSensor],
                bb->sensors[lFSRRearRightSensor]);

        foot_s rightFoot(
                bb->sensors[rFSRFrontLeftSensor],
                bb->sensors[rFSRFrontRightSensor],
                bb->sensors[rFSRRearLeftSensor],
                bb->sensors[rFSRRearRightSensor]);

        Foot fsr(leftFoot, rightFoot);

        YPR ypr(0.0f, bb->gyro(1), bb->gyro(0));
        ankleBalancer.proceed(ypr);

        Step step{};
        kick.proceed(bb, calib, &step, ankleBalancer, calib->stiffness);

        // SET ARMS ACTUATORS
        bb->actuators[lShoulderPitchPositionActuator] = step.angles[0];//calib->l_shoulder_pitch;
        bb->actuators[lShoulderRollPositionActuator] = step.angles[1];//calib->l_shoulder_roll;
        bb->actuators[lElbowYawPositionActuator] = step.angles[2];//step.angles[2];//calib->l_elbow_yaw;
        bb->actuators[lElbowRollPositionActuator] = step.angles[3];//calib->l_elbow_roll;

        bb->actuators[lWristYawPositionActuator] = -1.5f;
        bb->actuators[lHandPositionActuator] = 0.0f;

        bb->actuators[rShoulderPitchPositionActuator] = step.angles[18];//calib->r_shoulder_pitch;//bb->actuators[lShoulderPitchPositionActuator];
        bb->actuators[rShoulderRollPositionActuator] = step.angles[19];//calib->r_shoulder_roll;//-bb->actuators[lShoulderRollPositionActuator];
        bb->actuators[rElbowYawPositionActuator] = step.angles[20];//calib->r_elbow_yaw;//-bb->actuators[lElbowYawPositionActuator];
        bb->actuators[rElbowRollPositionActuator] = step.angles[21];//calib->r_elbow_roll;//-bb->actuators[lElbowRollPositionActuator];
        bb->actuators[rWristYawPositionActuator] = -bb->actuators[lWristYawPositionActuator];
        bb->actuators[rHandPositionActuator] = 0.0f;

        // SET LEGS ACTUATORS
        bb->actuators[lHipYawPitchPositionActuator] = step.angles[6];//calib->l_hip_yaw_pitch;
        bb->actuators[lHipRollPositionActuator] = step.angles[7];//calib->l_hip_roll;
        bb->actuators[lHipPitchPositionActuator] = step.angles[8];//calib->l_hip_pitch;
        bb->actuators[lKneePitchPositionActuator] = step.angles[9];//calib->l_knee_pitch;
        bb->actuators[lAnklePitchPositionActuator] = step.angles[10];//calib->l_ankle_pitch + step.angles[10];
        bb->actuators[lAnkleRollPositionActuator] = step.angles[11];//calib->l_ankle_roll + step.angles[11];

        bb->actuators[rHipRollPositionActuator] = step.angles[13];//calib->r_hip_roll;
        bb->actuators[rHipPitchPositionActuator] = step.angles[14];//calib->r_hip_pitch;
        bb->actuators[rKneePitchPositionActuator] = step.angles[15];//calib->r_knee_pitch;
        bb->actuators[rAnklePitchPositionActuator] = step.angles[16];//calib->r_ankle_pitch + step.angles[16];
        bb->actuators[rAnkleRollPositionActuator] = step.angles[17];//calib->r_ankle_roll + step.angles[17];

        // SET ARM STIFFNESS
        bb->actuators[lShoulderPitchHardnessActuator] = step.stiffnesses[0];
        bb->actuators[lShoulderRollHardnessActuator] = step.stiffnesses[1];
        bb->actuators[lElbowYawHardnessActuator] = step.stiffnesses[2];
        bb->actuators[lElbowRollHardnessActuator] = step.stiffnesses[3];
        
        bb->actuators[rShoulderPitchHardnessActuator] = step.stiffnesses[18];
        bb->actuators[rShoulderRollHardnessActuator] = step.stiffnesses[19];
        bb->actuators[rElbowYawHardnessActuator] = step.stiffnesses[20];
        bb->actuators[rElbowRollHardnessActuator] = step.stiffnesses[21];

        // SET LEGS STIFFNESS
        bb->actuators[lHipYawPitchHardnessActuator] = (step.stiffnesses[6] + step.stiffnesses[12])/2.0f;

        bb->actuators[lHipRollHardnessActuator] = step.stiffnesses[7];
        bb->actuators[lHipPitchHardnessActuator] = step.stiffnesses[8];
        bb->actuators[lKneePitchHardnessActuator] = step.stiffnesses[9];
        bb->actuators[lAnklePitchHardnessActuator] = step.stiffnesses[10];
        bb->actuators[lAnkleRollHardnessActuator] = step.stiffnesses[11];

        bb->actuators[rHipRollHardnessActuator] = step.stiffnesses[13];
        bb->actuators[rHipPitchHardnessActuator] = step.stiffnesses[14];
        bb->actuators[rKneePitchHardnessActuator] = step.stiffnesses[15];
        bb->actuators[rAnklePitchHardnessActuator] = step.stiffnesses[16];
        bb->actuators[rAnkleRollHardnessActuator] = step.stiffnesses[17];

        // SET LEGS STIFFNESS END
        return MOTION_UNSTABLE;
    }
    

private:
    MotionDesignBlackboard *calib;
    MotionDesignEngine kick;
    AnkleBalancer ankleBalancer;
};
