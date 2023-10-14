#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include <libbembelbots/bembelbots.h>


namespace lola::mapping {

using map_t = std::unordered_map<std::string, std::vector<int>>;

static const map_t sensors{
    {
        "Accelerometer",
        {
            accXSensor,
            accYSensor,
            accZSensor
        }
    },

    {
        "Angles",
        {
            angleXSensor,
            angleYSensor
            // no Z 
        }
    },

    {
        "Battery",
        {
            batteryChargeSensor,
            -1, // "status", missing in SHM
            batteryCurrentSensor,
            batteryTemperatureSensor
        }
    },

    {
        "Current",
        {
            headYawCurrentSensor,
            headPitchCurrentSensor,
            lShoulderPitchCurrentSensor,
            lShoulderRollCurrentSensor,
            lElbowYawCurrentSensor,
            lElbowRollCurrentSensor,
            lWristYawCurrentSensor,
            lHipYawPitchCurrentSensor,
            lHipRollCurrentSensor,
            lHipPitchCurrentSensor,
            lKneePitchCurrentSensor,
            lAnklePitchCurrentSensor,
            lAnkleRollCurrentSensor,
            rHipRollCurrentSensor,
            rHipPitchCurrentSensor,
            rKneePitchCurrentSensor,
            rAnklePitchCurrentSensor,
            rAnkleRollCurrentSensor,
            rShoulderPitchCurrentSensor,
            rShoulderRollCurrentSensor,
            rElbowYawCurrentSensor,
            rElbowRollCurrentSensor,
            rWristYawCurrentSensor,
            lHandCurrentSensor,
            rHandCurrentSensor
        }
    },

    {
        "FSR",
        {
            lFSRFrontLeftSensor,
            lFSRFrontRightSensor,
            lFSRRearLeftSensor,
            lFSRRearRightSensor,
            rFSRFrontLeftSensor,
            rFSRFrontRightSensor,
            rFSRRearLeftSensor,
            rFSRRearRightSensor
            // [rl]FSRTotalSensor removed in lola
        }
    },

    {
        "Gyroscope",
        {
            gyroXSensor,
            gyroYSensor,
            gyroZSensor
        }
    },

    {
        "Position",
        {
            headYawPositionSensor,
            headPitchPositionSensor,
            lShoulderPitchPositionSensor,
            lShoulderRollPositionSensor,
            lElbowYawPositionSensor,
            lElbowRollPositionSensor,
            lWristYawPositionSensor,
            lHipYawPitchPositionSensor,
            lHipRollPositionSensor,
            lHipPitchPositionSensor,
            lKneePitchPositionSensor,
            lAnklePitchPositionSensor,
            lAnkleRollPositionSensor,
            rHipRollPositionSensor,
            rHipPitchPositionSensor,
            rKneePitchPositionSensor,
            rAnklePitchPositionSensor,
            rAnkleRollPositionSensor,
            rShoulderPitchPositionSensor,
            rShoulderRollPositionSensor,
            rElbowYawPositionSensor,
            rElbowRollPositionSensor,
            rWristYawPositionSensor,
            lHandPositionSensor,
            rHandPositionSensor
        }
    },

    {
        "Sonar",
        {
            lUsSensor,
            rUsSensor
            // [rl]Us[1..9]Sensor removed in lola
        }
    },

    {
        "Status", // joint 
        {
            headYawStatusSensor,
            headPitchStatusSensor,
            lShoulderPitchStatusSensor,
            lShoulderRollStatusSensor,
            lElbowYawStatusSensor,
            lElbowRollStatusSensor,
            lWristYawStatusSensor,
            lHipYawPitchStatusSensor,
            lHipRollStatusSensor,
            lHipPitchStatusSensor,
            lKneePitchStatusSensor,
            lAnklePitchStatusSensor,
            lAnkleRollStatusSensor,
            rHipRollStatusSensor,
            rHipPitchStatusSensor,
            rKneePitchStatusSensor,
            rAnklePitchStatusSensor,
            rAnkleRollStatusSensor,
            rShoulderPitchStatusSensor,
            rShoulderRollStatusSensor,
            rElbowYawStatusSensor,
            rElbowRollStatusSensor,
            rWristYawStatusSensor,
            lHandStatusSensor,
            rHandStatusSensor
        }
    },

    {
        "Stiffness",   // missing in SHM, joint order same as position
        {
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
        }
    },

    {
        "Temperature",
        {
            headYawTemperatureSensor,
            headPitchTemperatureSensor,
            lShoulderPitchTemperatureSensor,
            lShoulderRollTemperatureSensor,
            lElbowYawTemperatureSensor,
            lElbowRollTemperatureSensor,
            lWristYawTemperatureSensor,
            lHipYawPitchTemperatureSensor,
            lHipRollTemperatureSensor,
            lHipPitchTemperatureSensor,
            lKneePitchTemperatureSensor,
            lAnklePitchTemperatureSensor,
            lAnkleRollTemperatureSensor,
            rHipRollTemperatureSensor,
            rHipPitchTemperatureSensor,
            rKneePitchTemperatureSensor,
            rAnklePitchTemperatureSensor,
            rAnkleRollTemperatureSensor,
            rShoulderPitchTemperatureSensor,
            rShoulderRollTemperatureSensor,
            rElbowYawTemperatureSensor,
            rElbowRollTemperatureSensor,
            rWristYawTemperatureSensor,
            lHandTemperatureSensor,
            rHandTemperatureSensor
        }
    },

    {
        "Touch",
        {
            chestButtonSensor,
            headTouchFrontSensor,
            headTouchMiddleSensor,
            headTouchRearSensor,
            lBumperLeftSensor,
            lBumperRightSensor,
            lHandTouchBackSensor,
            lHandTouchLeftSensor,
            lHandTouchRightSensor,
            rBumperLeftSensor,
            rBumperRightSensor,
            rHandTouchBackSensor,
            rHandTouchLeftSensor,
            rHandTouchRightSensor
        }
    },

    {
        "RobotConfig", // not handled by sensors array
        {
            -1, // Body/BodyId
            -1, // Body/Version
            -1, // Head/FullHeadId
            -1  // Head/Version
        }
    }
};



static const map_t actuators{
    {
        "Chest",
        {
            chestBoardLedRedActuator,
            chestBoardLedGreenActuator,
            chestBoardLedBlueActuator
        }
    },

    {
        "LEar",
        {
            earsLedLeft0DegActuator,
            earsLedLeft36DegActuator,
            earsLedLeft72DegActuator,
            earsLedLeft108DegActuator,
            earsLedLeft144DegActuator,
            earsLedLeft180DegActuator,
            earsLedLeft216DegActuator,
            earsLedLeft252DegActuator,
            earsLedLeft288DegActuator,
            earsLedLeft324DegActuator
        }
    },

    {
        "LEye",
        {
            faceLedRedLeft45DegActuator,
            faceLedRedLeft0DegActuator,
            faceLedRedLeft315DegActuator,
            faceLedRedLeft270DegActuator,
            faceLedRedLeft225DegActuator,
            faceLedRedLeft180DegActuator,
            faceLedRedLeft135DegActuator,
            faceLedRedLeft90DegActuator,
            faceLedGreenLeft45DegActuator,
            faceLedGreenLeft0DegActuator,
            faceLedGreenLeft315DegActuator,
            faceLedGreenLeft270DegActuator,
            faceLedGreenLeft225DegActuator,
            faceLedGreenLeft180DegActuator,
            faceLedGreenLeft135DegActuator,
            faceLedGreenLeft90DegActuator,
            faceLedBlueLeft45DegActuator,
            faceLedBlueLeft0DegActuator,
            faceLedBlueLeft315DegActuator,
            faceLedBlueLeft270DegActuator,
            faceLedBlueLeft225DegActuator,
            faceLedBlueLeft180DegActuator,
            faceLedBlueLeft135DegActuator,
            faceLedBlueLeft90DegActuator,
        }
    },

    {
        "LFoot",
        {
            lFootLedRedActuator,
            lFootLedGreenActuator,
            lFootLedBlueActuator
        }
    },

    {
        "Position",
        {
            headYawPositionActuator,
            headPitchPositionActuator,
            lShoulderPitchPositionActuator,
            lShoulderRollPositionActuator,
            lElbowYawPositionActuator,
            lElbowRollPositionActuator,
            lWristYawPositionActuator,
            lHipYawPitchPositionActuator,
            lHipRollPositionActuator,
            lHipPitchPositionActuator,
            lKneePitchPositionActuator,
            lAnklePitchPositionActuator,
            lAnkleRollPositionActuator,
            rHipRollPositionActuator,
            rHipPitchPositionActuator,
            rKneePitchPositionActuator,
            rAnklePitchPositionActuator,
            rAnkleRollPositionActuator,
            rShoulderPitchPositionActuator,
            rShoulderRollPositionActuator,
            rElbowYawPositionActuator,
            rElbowRollPositionActuator,
            rWristYawPositionActuator,
            lHandPositionActuator,
            rHandPositionActuator
        }
    },

    {
        "REar",
        {
            earsLedRight0DegActuator,
            earsLedRight36DegActuator,
            earsLedRight72DegActuator,
            earsLedRight108DegActuator,
            earsLedRight144DegActuator,
            earsLedRight180DegActuator,
            earsLedRight216DegActuator,
            earsLedRight252DegActuator,
            earsLedRight288DegActuator,
            earsLedRight324DegActuator
        }
    },

    {
        "REye",
        {
            faceLedRedRight45DegActuator,
            faceLedRedRight0DegActuator,
            faceLedRedRight315DegActuator,
            faceLedRedRight270DegActuator,
            faceLedRedRight225DegActuator,
            faceLedRedRight180DegActuator,
            faceLedRedRight135DegActuator,
            faceLedRedRight90DegActuator,
            faceLedGreenRight45DegActuator,
            faceLedGreenRight0DegActuator,
            faceLedGreenRight315DegActuator,
            faceLedGreenRight270DegActuator,
            faceLedGreenRight225DegActuator,
            faceLedGreenRight180DegActuator,
            faceLedGreenRight135DegActuator,
            faceLedGreenRight90DegActuator,
            faceLedBlueRight45DegActuator,
            faceLedBlueRight0DegActuator,
            faceLedBlueRight315DegActuator,
            faceLedBlueRight270DegActuator,
            faceLedBlueRight225DegActuator,
            faceLedBlueRight180DegActuator,
            faceLedBlueRight135DegActuator,
            faceLedBlueRight90DegActuator,
        }
    },

    {
        "RFoot",
        {
            rFootLedRedActuator,
            rFootLedGreenActuator,
            rFootLedBlueActuator
        }
    },

    {
        "Skull",
        {
            headLedFrontLeft1Actuator,
            headLedFrontLeft0Actuator,
            headLedMiddleLeft0Actuator,
            headLedRearLeft0Actuator,
            headLedRearLeft1Actuator,
            headLedRearLeft2Actuator,
            headLedRearRight2Actuator,
            headLedRearRight1Actuator,
            headLedRearRight0Actuator,
            headLedMiddleRight0Actuator,
            headLedFrontRight0Actuator,
            headLedFrontRight1Actuator
        }
    },

    {
        "Sonar",
        {
            lUsSensor, // left
            rUsSensor  // right
        }
    },

    {
        "Stiffness",
        {
            headYawHardnessActuator,
            headPitchHardnessActuator,
            lShoulderPitchHardnessActuator,
            lShoulderRollHardnessActuator,
            lElbowYawHardnessActuator,
            lElbowRollHardnessActuator,
            lWristYawHardnessActuator,
            lHipYawPitchHardnessActuator,
            lHipRollHardnessActuator,
            lHipPitchHardnessActuator,
            lKneePitchHardnessActuator,
            lAnklePitchHardnessActuator,
            lAnkleRollHardnessActuator,
            rHipRollHardnessActuator,
            rHipPitchHardnessActuator,
            rKneePitchHardnessActuator,
            rAnklePitchHardnessActuator,
            rAnkleRollHardnessActuator,
            rShoulderPitchHardnessActuator,
            rShoulderRollHardnessActuator,
            rElbowYawHardnessActuator,
            rElbowRollHardnessActuator,
            rWristYawHardnessActuator,
            lHandHardnessActuator,
            rHandHardnessActuator
        }
    }
};

}


// vim: set ts=4 sw=4 sts=4 expandtab:
