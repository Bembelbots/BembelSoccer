#include "actuatorcheck.h"

#include <cmath>

#include <libbembelbots/bembelbots.h>
#include <framework/joints/definitionsBody.h>

inline bool forceActuatorRange(float & value, const float constraint[2]){
    if (value < constraint[0]){
        value = constraint[0];
        return false;
    }

    if (value > constraint[1]){
        value = constraint[1];
        return false;
    }

    if (std::isnormal(value)){
        return true;
    }

    if (std::isnan(value)){
        return false;
    }

    return true;
}

bool checkAndCorrectActuators(float *actuators, float *sensors){
    bool return_code = true;
    return_code &= forceActuatorRange(actuators[headYawPositionActuator],HEAD_YAW_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[headPitchPositionActuator],HEAD_PITCH_CONSTRAINTS);

    return_code &= forceActuatorRange(actuators[lShoulderPitchPositionActuator],L_SHOULDER_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lShoulderRollPositionActuator],L_SHOULDER_ROLL_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lElbowYawPositionActuator],L_ELBOW_YAW_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lElbowRollPositionActuator],L_ELBOW_ROLL_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lWristYawPositionActuator],L_WRIST_YAW_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lHandPositionActuator],L_HAND_CONSTRAINTS);

    return_code &= forceActuatorRange(actuators[lHipYawPitchPositionActuator],L_HIP_YAW_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lHipRollPositionActuator],L_HIP_ROLL_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lHipPitchPositionActuator],L_HIP_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lKneePitchPositionActuator],L_KNEE_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lAnklePitchPositionActuator],L_ANKLE_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[lAnkleRollPositionActuator],L_ANKLE_ROLL_CONSTRAINTS);

    return_code &= forceActuatorRange(actuators[rShoulderPitchPositionActuator],R_SHOULDER_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rShoulderRollPositionActuator],R_SHOULDER_ROLL_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rElbowYawPositionActuator],R_ELBOW_YAW_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rElbowRollPositionActuator],R_ELBOW_ROLL_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rWristYawPositionActuator],R_WRIST_YAW_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rHandPositionActuator],R_HAND_CONSTRAINTS);

    /* return_code &= forceActuatorRange(actuators[rHipYawPitchPositionActuator],R_HIP_YAW_PITCH_CONSTRAINTS); */
    return_code &= forceActuatorRange(actuators[rHipRollPositionActuator],R_HIP_ROLL_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rHipPitchPositionActuator],R_HIP_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rKneePitchPositionActuator],R_KNEE_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rAnklePitchPositionActuator],R_ANKLE_PITCH_CONSTRAINTS);
    return_code &= forceActuatorRange(actuators[rAnkleRollPositionActuator],R_ANKLE_ROLL_CONSTRAINTS);

    return return_code;
}

