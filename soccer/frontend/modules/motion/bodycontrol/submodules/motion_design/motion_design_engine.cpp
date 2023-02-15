#include "motion_design_engine.h"

#include <bodycontrol/blackboards/motion_design_blackboard.h>

#include <cstdio>
#include <vector>

#include <representations/bembelbots/constants.h>

MotionDesignEngine::MotionDesignEngine(){

}
MotionDesignEngine::~MotionDesignEngine() {}


void MotionDesignEngine::proceed(BodyBlackboard *bb, MotionDesignBlackboard *calib, Step *step, const AnkleBalancer &ankle_balancer, float &stiffnessVal){
	if(calib->reReadActuators){
		calib->reReadActuators = false;
		current.read(bb->sensors);
		write_to_blackboard(calib);
		return;
	}

	if(calib->isPlayKeyframe){
		//std::cout << "playKeyframe..." << std::endl;
		//save start_time
		if(start_time < bb->timestamp_ms - calib->time){
			//std::cout << "start time " << start_time << std::endl;
			//std::cout << "end time " << bb->timestamp_ms - calib->time << std::endl;
			start_time = bb->timestamp_ms;
			start.read(bb->sensors);
			current.read(bb->sensors);
			//create temporal joint 
			/*
			joints::pos::Old tempJoints({
				0.0f, // HEAD_YAW 0
				0.34f, // HEAD_PITCH 0 
				
	            calib->l_shoulder_pitch, // LEFT_SHOULDER_PITCH 2
	            calib->l_shoulder_roll, // LEFT_SHOULDER_ROLL 3
	            calib->l_elbow_yaw, // LEFT_ELBOW_YAW 4 
	            calib->l_elbow_roll, // LEFT_ELBOW_ROLL 5

	            calib->r_shoulder_pitch, // RIGHT_SHOULDER_PITCH 6
	            calib->r_shoulder_roll, // RIGHT_SHOULDER_ROLL 7
	            calib->r_elbow_yaw, // RIGHT_ELBOW_YAW 8
	            calib->r_elbow_roll, // RIGHT_ELBOW_ROLL 9

	            calib->l_hip_yaw_pitch, // HIP_YAW_PITCH 10

	            calib->l_hip_roll, // LEFT_HIP_ROLL 11
	            calib->l_hip_pitch, // LEFT_HIP_PITCH 12
	            calib->l_knee_pitch, // LEFT_KNEE_PITCH 13
	            calib->l_ankle_pitch, // LEFT_ANKEL_PITCH 14
	            calib->l_ankle_roll, // LEFT_ANKEL_ROLL 15

	            // 12 left out due to double HIP_YAW_PITCH
	            calib->r_hip_roll, // RIGHT_HIP_ROLL 16
	            calib->r_hip_pitch, // RIGHT_HIP_PITCH 17
	            calib->r_knee_pitch, // RIGHT_KNEE_PITCH 18
	            calib->r_ankle_pitch, // RIGHT_ANKEL_PITCH 19
	            calib->r_ankle_roll, // RIGHT_ANKEL_ROLL 20
			});
			*/
			joints::pos::Old tempJoints(joints::Joints{
                .headYaw = 0.f, // HEAD_YAW
                .headPitch = CONST::initial_pitch, // HEAD_PITCH
                .lShoulderPitch = calib->l_shoulder_pitch, // LEFT_SHOULDER_PITCH
                .lShoulderRoll = calib->l_shoulder_roll, // LEFT_SHOULDER_ROLL
                .lElbowYaw = calib->l_elbow_yaw, // LEFT_ELBOW_YAW
                .lElbowRoll = calib->l_elbow_roll, // LEFT_ELBOW_ROLL
                .lWristYaw = 0,
                .lHand = 0,

                .hipYawPitch = calib->l_hip_yaw_pitch, // HIP_YAW_PITCH

                .lHipRoll = calib->l_hip_roll, // LEFT_HIP_ROLL
                .lHipPitch = calib->l_hip_pitch, // LEFT_HIP_PITCH
                .lKneePitch = calib->l_knee_pitch, // LEFT_KNEE_PITCH
                .lAnklePitch = calib->l_ankle_pitch, // LEFT_ANKEL_PITCH
                .lAnkleRoll = calib->l_ankle_roll, // LEFT_ANKEL_ROLL

                .rShoulderPitch = calib->r_shoulder_pitch, // RIGHT_SHOULDER_PITCH
                .rShoulderRoll = calib->r_shoulder_roll, // RIGHT_SHOULDER_ROLL

                .rElbowYaw = calib->r_elbow_yaw, // RIGHT_ELBOW_YAW
                .rElbowRoll = calib->r_elbow_roll, // RIGHT_ELBOW_ROLL

                .rWristYaw = 0,
                .rHand = 0,

                // 12 left out due to double HIP_YAW_PITCH

                .rHipRoll = calib->r_hip_roll, // RIGHT_HIP_ROLL
                .rHipPitch = calib->r_hip_pitch, // RIGHT_HIP_PITCH
                .rKneePitch = calib->r_knee_pitch, // RIGHT_KNEE_PITCH
                .rAnklePitch = calib->r_ankle_pitch, // RIGHT_ANKEL_PITCH
                .rAnkleRoll = calib->r_ankle_roll, // RIGHT_ANKEL_ROLL
            });
			target = tempJoints;
			calib->isPlayKeyframe = false;
		}

	} else if(calib->isPlayAllKeyframes && isNewFrame){ // if all keyframes should be played
		if(interpolationPercentage >= 1.f){
			// Callback for Debug to send next keyframe
			std::cout << "INTERPOLATION IS COMPLETE" << std::endl;
			calib->isKeyframeInterpolationComplete = true;
			calib->isPlayAllKeyframes = false;
			isNewFrame = false;
		} 
		//interpolationPercentage = std::max(0.0f, std::min(1.0f, float(bb->timestamp_ms - start_time) / float(calib->time)));

	} else if(interpolationPercentage >= 1.f){ //0.99.f // if interpolation is done
		// This is for setting keyframes by stiffness based approach
		target = current;
	}
	
	interpolationPercentage = std::max(0.0f, std::min(1.0f, float(bb->timestamp_ms - start_time) / float(calib->time)));
	//std::cout << "interpolationPercentage: " << interpolationPercentage << std::endl;
	set_joints(step, ankle_balancer, true);
	set_stiffness(step, calib);
}

void MotionDesignEngine::set_joints(Step *step, const AnkleBalancer &ankle_balancer, bool isInterpolating){
	// interpolate joints
	if(isInterpolating){
        current.each([&](joint_id i) { current[i] = interpolate_joint(start[i], target[i], interpolationPercentage); });
    }
	
	//ARMS
    step->angles[0] = current[L_SHOULDER_PITCH];  //leftShoulderPitch
    step->angles[1] = current[L_SHOULDER_ROLL];   //leftShoulderRoll
    step->angles[2] = current[L_ELBOW_YAW];       //leftElbowYawPosition
    step->angles[3] = current[L_ELBOW_ROLL];      //leftElbowRoll
    step->angles[18] = current[R_SHOULDER_PITCH]; //rightShoulderPitch
    step->angles[19] = current[R_SHOULDER_ROLL];  //rShoulderRoll
    step->angles[20] = current[R_ELBOW_YAW];      //rightElbowYaw
    step->angles[21] = current[R_ELBOW_ROLL];     //rightElbowRoll

    //LEGS
    step->angles[6] = current[L_HIP_YAW_PITCH];  //j.hipYawPitch;
    step->angles[7] = current[L_HIP_ROLL];       //j.leftLeg.hipRoll;
    step->angles[8] = current[L_HIP_PITCH];      //j.leftLeg.hipPitch;
    step->angles[9] = current[L_KNEE_PITCH];     //j.leftLeg.kneePitch;
    step->angles[10] = current[L_ANKLE_PITCH];   //j.leftLeg.anklePitch;
    step->angles[11] = current[L_ANKLE_ROLL];    //j.leftLeg.ankleRoll ;
    step->angles[12] = current[L_HIP_YAW_PITCH]; //j.hipYawPitch;
    step->angles[13] = current[R_HIP_ROLL];      //j.rightLeg.hipRoll ;
    step->angles[14] = current[R_HIP_PITCH];     //j.rightLeg.hipPitch;
    step->angles[15] = current[R_KNEE_PITCH];    //j.rightLeg.kneePitch;
    step->angles[16] = current[R_ANKLE_PITCH];   //j.rightLeg.anklePitch;
    step->angles[17] = current[R_ANKLE_ROLL];    //j.rightLeg.ankleRoll;

    //ANKLE BALANCER
	step->angles[10] += ankle_balancer.gyroPitch * 0.5f;
    step->angles[11] += ankle_balancer.gyroRoll * 0.5f;
    step->angles[16] += ankle_balancer.gyroPitch * 0.5;
    step->angles[17] += ankle_balancer.gyroRoll * 0.5;

}

void MotionDesignEngine::set_stiffness(Step *step, MotionDesignBlackboard* calib){
    step->stiffnesses[0] = calib->lShoulderPitchStiffness;
    step->stiffnesses[1] = calib->lShoulderRollStiffness;
	step->stiffnesses[2] = calib->lElbowYawStiffness;
	step->stiffnesses[3] = calib->lElbowRollStiffness;

	step->stiffnesses[6] = calib->lHipYawPitchStiffness;
	step->stiffnesses[7] = calib->lHipRollStiffness;
	step->stiffnesses[8] = calib->lHipPitchStiffness;
	step->stiffnesses[9] = calib->lKneePitchStiffness;
	step->stiffnesses[10] = calib->lAnklePitchStiffness;
	step->stiffnesses[11] = calib->lAnkleRollStiffness;
	
	step->stiffnesses[13] = calib->rHipRollStiffness;
	step->stiffnesses[14] = calib->rHipPitchStiffness;
	step->stiffnesses[15] = calib->rKneePitchStiffness;
	step->stiffnesses[16] = calib->rAnklePitchStiffness;
	step->stiffnesses[17] = calib->rAnkleRollStiffness;
	step->stiffnesses[18] = calib->rShoulderPitchStiffness;
	step->stiffnesses[19] = calib->rShoulderRollStiffness;
	step->stiffnesses[20] = calib->rElbowYawStiffness;
	step->stiffnesses[21] = calib->rElbowRollStiffness;

}

float MotionDesignEngine::interpolate_joint(float current, float target, float percentage){
	return current * (1-percentage) + target * percentage;
}

void MotionDesignEngine::write_to_blackboard(MotionDesignBlackboard* calib){
    calib->l_shoulder_pitch = current[L_SHOULDER_PITCH]; // LEFT_SHOULDER_PITCH 2
    calib->l_shoulder_roll = current[L_SHOULDER_ROLL];   // LEFT_SHOULDER_ROLL 3
    calib->l_elbow_yaw = current[L_ELBOW_YAW];           // LEFT_ELBOW_YAW 4
    calib->l_elbow_roll = current[L_ELBOW_ROLL];         // LEFT_ELBOW_ROLL 5

    calib->r_shoulder_pitch = current[R_SHOULDER_PITCH]; // RIGHT_SHOULDER_PITCH 6
    calib->r_shoulder_roll = current[R_SHOULDER_ROLL];   // RIGHT_SHOULDER_ROLL 7
    calib->r_elbow_yaw = current[R_ELBOW_YAW];           // RIGHT_ELBOW_YAW 8
    calib->r_elbow_roll = current[R_ELBOW_ROLL];         // RIGHT_ELBOW_ROLL 9

    calib->l_hip_yaw_pitch = current[L_HIP_YAW_PITCH]; // HIP_YAW_PITCH 10

    calib->l_hip_roll = current[L_HIP_ROLL];       // LEFT_HIP_ROLL 11
    calib->l_hip_pitch = current[L_HIP_PITCH];     // LEFT_HIP_PITCH 12
    calib->l_knee_pitch = current[L_KNEE_PITCH];   // LEFT_KNEE_PITCH 13
    calib->l_ankle_pitch = current[L_ANKLE_PITCH]; // LEFT_ANKEL_PITCH 14
    calib->l_ankle_roll = current[L_ANKLE_ROLL];   // LEFT_ANKEL_ROLL 15

    calib->r_hip_roll = current[R_HIP_ROLL];       // RIGHT_HIP_ROLL 16
    calib->r_hip_pitch = current[R_HIP_PITCH];     // RIGHT_HIP_PITCH 17
    calib->r_knee_pitch = current[R_KNEE_PITCH];   // RIGHT_KNEE_PITCH 18
    calib->r_ankle_pitch = current[R_ANKLE_PITCH]; // RIGHT_ANKEL_PITCH 19
    calib->r_ankle_roll = current[R_ANKLE_ROLL];   // RIGHT_ANKEL_ROLL 20
}
