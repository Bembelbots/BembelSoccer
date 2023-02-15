/** @author: Jonathan Cyriax Brast , Bembelbots Team Frankfurt
 *  This file:
 *  Defines the masses , CoM positions and links between body parts of a Nao H21 model robot
 *
 * mass and com taken from:
 * https://community.aldebaran-robotics.com/doc/1-14/family/robots/massesrobot.html
 * https://community.aldebaran-robotics.com/doc/1-14/family/naoh21/masses_h21.html
 * links taken from:
 * https://community.aldebaran-robotics.com/doc/1-14/family/naoh21/links_h21.html
 *
 */
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//                                 //
// WARNING WARNING WARNING WARNING //
//                                 //
// The definitions are in [mm] but //
// the rest of the framework uses  //
// SI units of [m]                 //
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//


#ifndef DEFINITIONS_KINEMATICS_BODY_
#define DEFINITIONS_KINEMATICS_BODY_

#include <map>
#include <string>


const int DEGREES_OF_FREEDOM = 25;
const int NUMBER_OF_SENSORS = 26;
const int NUMBER_OF_ACTUATORS = 26;
const int NR_OF_ACTUATORS = NUMBER_OF_ACTUATORS;
const int NR_OF_SENSORS = NUMBER_OF_SENSORS;

const int NO_AXIS = 0;
const int X_AXIS = 1;
const int ROLL = 1;
const int Y_AXIS = 2;
const int PITCH = 2;
const int Z_AXIS = 4;
const int YAW = 4;
const int YZ_AXIS = 8;
const int MIRROR_AXIS = 16;

const float JOINT_SENSOR_PRECISION = (1.0f/4096.0f);
const float JOINT_PRECISION = (1.0f/4096.0f);
const float JOINT_SENSOR_COMPARE = (4.0f/4096.0f);
const float JOINT_COMPARE = (4.0f/4096.0f);

enum joint_id {
    JOINTS_BEGIN = 0,
    HEAD_YAW = JOINTS_BEGIN,
    HEAD_PITCH,

    L_SHOULDER_PITCH,
    L_SHOULDER_ROLL,

    L_ELBOW_YAW,
    L_ELBOW_ROLL,

    L_WRIST_YAW,
    L_HAND,

    L_HIP_YAW_PITCH,
    R_HIP_YAW_PITCH = L_HIP_YAW_PITCH,

    L_HIP_ROLL,
    L_HIP_PITCH,

    L_KNEE_PITCH,

    L_ANKLE_PITCH,
    L_ANKLE_ROLL,

    R_SHOULDER_PITCH,
    R_SHOULDER_ROLL,
    R_ELBOW_YAW,
    R_ELBOW_ROLL,
    R_WRIST_YAW,

    R_HAND,

    R_HIP_ROLL,
    R_HIP_PITCH,

    R_KNEE_PITCH,

    R_ANKLE_PITCH,
    R_ANKLE_ROLL,

    NR_OF_JOINTS,
    JOINTS_END = NR_OF_JOINTS,
};

inline constexpr joint_id &operator++(joint_id &id, int) {
    id = static_cast<joint_id>(id + 1);
    return id;
}

static_assert(NR_OF_JOINTS == DEGREES_OF_FREEDOM,
              "Number of Joints must be equal to Degrees of freedom");

namespace kin {
static const std::map<joint_id, std::string> IdNameTable{
    {HEAD_YAW, "HeadYaw"},
    {HEAD_PITCH, "HeadPitch"},
    {L_SHOULDER_PITCH, "LShoulderPitch"},
    {L_SHOULDER_ROLL, "LShoulderRoll"},
    {L_ELBOW_YAW, "LElbowYaw"},
    {L_ELBOW_ROLL, "LElbowRoll"},
    {L_HIP_YAW_PITCH, "HipYawPitch"},
    {L_HIP_ROLL, "LHipRoll"},
    {L_WRIST_YAW, "LWristYaw"},
    {L_HAND, "LHand"},
    {L_HIP_PITCH, "LHipPitch"},
    {L_KNEE_PITCH, "LKneePitch"},
    {L_ANKLE_PITCH, "LAnklePitch"},
    {L_ANKLE_ROLL, "LAnkleRoll"},
    {R_SHOULDER_PITCH, "RShoulderPitch"},
    {R_SHOULDER_ROLL, "RShoulderRoll"},
    {R_ELBOW_YAW, "RElbowYaw"},
    {R_ELBOW_ROLL, "RElbowRoll"},
    {R_WRIST_YAW, "RWristYaw"},
    {R_HAND, "RHand"},
    {R_HIP_ROLL, "RHipRoll"},
    {R_HIP_PITCH, "RHipPitch"},
    {R_KNEE_PITCH, "RKneePitch"},
    {R_ANKLE_PITCH, "RAnklePitch"},
    {R_ANKLE_ROLL, "RAnkleRoll"}
};
}

struct joint_definition {
    const float *link;
    const float *com;
    const float *constraints;
    const float *inertia;
    const int axis;
};


const float         TORSO_LINK[] = { 0.0, 0.0, 0.0 };
const float         TORSO_COM[] = { 1.04956 -0.00413, 0.00009, 0.04342 };   // _T_O_R_S_O
const float         TORSO_INERTIA[] = { 0.00506234058, 0.00001431158, 0.00015519082, 0.00488013591, -0.0002707934, 0.00161030006 };
//struct joint_definition TORSO_DEFINITION = { TORSO_LINK, TORSO_COM, NULL, TORSO_INERTIA, NO_AXIS };

const float         BATTERY_LINK[] = { 0.0, 0.0, 0.0 };
const float         BATTERY_COM[] = { 0.000 -0.03, 0.0, 0.039 };
//struct joint_definition BATTERY_DEFINITION = { BATTERY_LINK, BATTERY_COM, NULL, NULL, NO_AXIS };


// HEAD
const float        NECK_OFFSET_Z = 126.5;

const float         HEAD_YAW_LINK[] = { 0.0, 0.0, NECK_OFFSET_Z };
const float         HEAD_YAW_COM[] = { 0.06442 -0.00001, 0.00014, -0.02742 };   // _N_E_C_K
const float         HEAD_YAW_CONSTRAINTS[] = { -2.0857, 2.0857 };
const float         HEAD_YAW_INERTIA[] = { 0.00007499295, 0.00000000157, -0.00000001834, 0.00007599995, -0.0000005295, 0.0000553373 };
//struct joint_definition HEAD_YAW_DEFINITION = { HEAD_YAW_LINK, HEAD_YAW_COM, HEAD_YAW_CONSTRAINTS, HEAD_YAW_INERTIA, YAW };

const float         HEAD_PITCH_LINK[] = { 0.0, 0.0, 0.0 };
const float         HEAD_PITCH_COM[] = { 0.60533 -0.00112, 0.00003, 0.05258 };   // _H_E_A_D
const float         HEAD_PITCH_CONSTRAINTS[] = { -0.6720, 0.5149 };
const float         HEAD_PITCH_INERTIA[] = { 0.00263129518, 0.0000878814, 0.00004098466, 0.00249112488, -0.00002995792, 0.00098573565 };


// ARMS

const float        SHOULDER_OFFSET_Y = 98.0;
const float        SHOULDER_OFFSET_Z = 100.0;
const float        ELBOW_OFFSET_Y = 15.0;
const float        UPPER_ARM_LENGTH = 105.0;
const float        LOWER_ARM_LENGTH = 55.95;
const float        HAND_OFFSET_X = 57.75;
const float        HAND_OFFSET_Z = 12.31;



// LEFT ARM
const float         L_SHOULDER_PITCH_LINK[] = { 0.0, SHOULDER_OFFSET_Y, SHOULDER_OFFSET_Z };
const float         L_SHOULDER_PITCH_COM[] = { 0.07504 -0.00165, -0.02663, 0.00014 };   // _S_H_O_U_L_D_E_R
const float         L_SHOULDER_PITCH_CONSTRAINTS[] = { -2.0857, 2.0857 };
const float         L_SHOULDER_PITCH_INERTIA[] = { 0.00008428430, -0.00000202802, 0.00000002338, 0.00001415561, -0.00000001972, 0.00008641949 };

const float         L_SHOULDER_ROLL_LINK[] = { 0.0, 0.0, 0.0 };
const float         L_SHOULDER_ROLL_COM[] = { 0.15777, 0.02455, 0.00563, 0.00330 };    // _B_I_C_E_P
const float         L_SHOULDER_ROLL_CONSTRAINTS[] = { -0.3142, 1.3265 };
const float         L_SHOULDER_ROLL_INERTIA[] = { 0.00009389993, -0.00004714452, -0.00002699471, 0.00037151879, -0.00000245977, 0.000341990083 };

const float         L_ELBOW_YAW_LINK[] = { UPPER_ARM_LENGTH, ELBOW_OFFSET_Y, 0.0 };
const float         L_ELBOW_YAW_COM[] = { 0.06483 -0.02744, 0.00000, -0.00014 };   // _E_L_B_O_W
const float         L_ELBOW_YAW_CONSTRAINTS[] = { -2.0857, 2.0857 };
const float         L_ELBOW_YAW_INERTIA[] = { 0.00000559715, 0.00000000421, 0.00000004319, 0.00007543312, -0.00000000184, 0.00007644339 };

const float         L_ELBOW_ROLL_LINK[] = { 0.0, 0.0, 0.0 };
const float         L_ELBOW_ROLL_COM[] = { 0.07761, 0.02556, 0.00281, 0.00076 };    // _U_N_M_O_T_O_R_I_E_D _F_O_R_E_A_R_M _A_N_D _H_A_N_D
const float         L_ELBOW_ROLL_CONSTRAINTS[] = { -1.5446, -0.0349 };
const float         L_ELBOW_ROLL_INERTIA[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

/* const float         L_ELBOW_ROLL_LINK21[] = { 0.0, 0.0, 0.0 };
const float         L_ELBOW_ROLL_COM21[] = { 0.18405, 0.06530, 0.00114, 0.00051 };  // _U_N_M_O_T_O_R_I_E_D _F_O_R_E_A_R_M _A_N_D _H_A_N_D
const float         L_ELBOW_ROLL_CONSTRAINTS21[] = { -1.5446, -0.0349 };
const float         L_ELBOW_ROLL_INERTIA21[] = { 0.00007810423, -0.00001233383, 0.00000744169, 0.00112084378, -0.00000054551, 0.00112214306 }; */

const float         L_WRIST_YAW_LINK[] = { LOWER_ARM_LENGTH, 0.0, 0.0 };
const float         L_WRIST_YAW_COM[] = { 0.18533, 0.03434, -0.00088, 0.00308 };
const float         L_WRIST_YAW_CONSTRAINTS[] = { -1.8238, 1.8238 };
const float         L_WRIST_YAW_INERTIA[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

const float         L_HAND_LINK[] = { HAND_OFFSET_X, 0.0, HAND_OFFSET_Z };
const float         L_HAND_COM[] = { 0.0, 0.0, 0.0, 0.0 };
const float         L_HAND_CONSTRAINTS[] = { -3.14, 3.14 };  // WHATEVER / OPEN AND CLOSE ARE NOT USEFUL ANGLES
const float         L_HAND_INERTIA[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };



// RIGHT ARM

const float         R_SHOULDER_PITCH_LINK[] = { 0.0, -SHOULDER_OFFSET_Z, SHOULDER_OFFSET_Z };
const float         R_SHOULDER_PITCH_COM[] = { 0.07504, -0.00165, 0.02663, 0.00014 };    // _S_H_O_U_L_D_E_R
const float         R_SHOULDER_PITCH_CONSTRAINTS[] = { -2.0857, 2.0857 };
const float         R_SHOULDER_PITCH_INERTIA[] = { 0.00008428430, 0.00000202802, 0.00000002338, 0.00001415561, 0.00000001972, 0.00008641949 };

const float         R_SHOULDER_ROLL_LINK[] = { 0.0, 0.0, 0.0 };
const float         R_SHOULDER_ROLL_COM[] = { 0.15794, 0.02429, -0.00952, 0.00320 };    // _B_I_C_E_P
const float         R_SHOULDER_ROLL_CONSTRAINTS[] = { -1.3265, 0.3142 };
const float         R_SHOULDER_ROLL_INERTIA[] = { 0.00011012031, 0.00007669131, -0.00002604607, 0.00036757653, 0.00001209828, 0.0035461772 };

const float         R_ELBOW_YAW_LINK[] = { UPPER_ARM_LENGTH, -ELBOW_OFFSET_Y, 0.0 };
const float         R_ELBOW_YAW_COM[] = { 0.06483 -0.02744, 0.00000, -0.00014 };   // _E_L_B_O_W
const float         R_ELBOW_YAW_CONSTRAINTS[] = { -2.0875, 2.0875 };
const float         R_ELBOW_YAW_INERTIA[] = { 0.00000559715, 0.00000000421, 0.00000004319, 0.00007543312, -0.00000000184, 0.00007644339 };

const float         R_ELBOW_ROLL_LINK[] = { 0.0, 0.0, 0.0 };
const float         R_ELBOW_ROLL_COM[] = { 0.07761, 0.02556, -0.00281, 0.00076 };    // _U_N_M_O_T_O_R_I_E_D _F_O_R_E_A_R_M _A_N_D _H_A_N_D
const float         R_ELBOW_ROLL_CONSTRAINTS[] = { 0.0349, 1.5446};
const float         R_ELBOW_ROLL_INERTIA[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

const float         R_WRIST_YAW_LINK[] = { LOWER_ARM_LENGTH, 0.0, 0.0 };
const float         R_WRIST_YAW_COM[] = { 0.18533, 0.03434, 0.00088, 0.00308 };
const float         R_WRIST_YAW_CONSTRAINTS[] = { -1.8238, 1.8238 };
const float         R_WRIST_YAW_INERTIA[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

const float         R_HAND_LINK[] = { HAND_OFFSET_X, 0.0, HAND_OFFSET_Z };
const float         R_HAND_COM[] = { 0.0, 0.0, 0.0, 0.0 };
const float         R_HAND_CONSTRAINTS[] = { -3.14, 3.14 };  // WHATEVER / OPEN AND CLOSE ARE NOT USEFUL ANGLES
const float         R_HAND_INERTIA[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

// HIPS
const float        HIP_OFFSET_Y = 50.0;
const float        HIP_OFFSET_Z = 85.0;

const float         L_HIP_YAW_PITCH_LINK[] = { 0.0,  HIP_OFFSET_Y, -HIP_OFFSET_Z };
const float         L_HIP_YAW_PITCH_COM[] = { 0.06981 -0.00781, -0.01114, 0.02661 };   // _P_E_L_V_I_S
const float         L_HIP_YAW_PITCH_CONSTRAINTS[] = { -1.145303, 0.740810 };
const float         L_HIP_YAW_PITCH_INERTIA[] = { 0.00008150233, -0.00000499449, 0.00001274817, 0.00010132555, 0.00002345474, 0.00006262363 };

const float         R_HIP_YAW_PITCH_LINK[] = { 0.0, -HIP_OFFSET_Y, -HIP_OFFSET_Z };
const float         R_HIP_YAW_PITCH_COM[] = { 0.07118, -0.00766, 0.01200, 0.02716 };    // _P_E_L_V_I_S
const float         R_HIP_YAW_PITCH_CONSTRAINTS[] = { -1.145303, 0.740810 };
const float         R_HIP_YAW_PITCH_INERTIA[] = { 0.00008997195, 0.00000500219, 0.00001273525, 0.00010552611, -0.00002770080, 0.00006688724 };

// LEGS
const float        THIGH_LENGTH = 100.0;
const float        TIBIA_LENGTH = 102.9;
const float        FOOT_HEIGHT = 45.19;

// LEFT LEG
const float         L_HIP_ROLL_LINK[] = { 0.0, 0.0, 0.0 };
const float         L_HIP_ROLL_COM[] = { 0.13053, -0.01549, 0.00029, -0.00515 };    // _H_I_P
const float         L_HIP_ROLL_CONSTRAINTS[] = { -0.379472, 0.790477 };
const float         L_HIP_ROLL_INERTIA[] = { 0.000027583544, -0.00000002233, 0.00000408164, 0.00009827055, 0.00000000419, 0.00008809973 };

const float         L_HIP_PITCH_LINK[] = { 0.0, 0.0, 0.0 };
const float         L_HIP_PITCH_COM[] = { 0.38968, 0.00139, 0.00221, -0.05373 };    // _T_I_G_H_T
const float         L_HIP_PITCH_CONSTRAINTS[] = { -1.535889, 0.484090 };
const float         L_HIP_PITCH_INERTIA[] = { 0.00163671962, 0.00000092451, 0.00008530668, 0.00159107278, 0.00030374342, 0.00030374342 };

const float         L_KNEE_PITCH_LINK[] = { 0.0, 0.0, -THIGH_LENGTH };
const float         L_KNEE_PITCH_COM[] = { 0.29142, 0.00453, 0.00225, -0.04936 };    // _T_I_B_I_A
const float         L_KNEE_PITCH_CONSTRAINTS[] = { -0.092346, 2.112528 };
const float         L_KNEE_PITCH_INERTIA[] = { 0.00118207967, 0.00000063362, 0.00003649697, 0.00112865226, 0.00003949523, 0.0019322744 };

const float         L_ANKLE_PITCH_LINK[] = { 0.0, 0.0, -TIBIA_LENGTH };
const float         L_ANKLE_PITCH_COM[] = { 0.13416, 0.00045, 0.00029, 0.00685 };    // _A_N_K_L_E
const float         L_ANKLE_PITCH_CONSTRAINTS[] = { -1.189516, 0.922747 };
const float         L_ANKLE_PITCH_INERTIA[] = { 0.00003850978, -0.00000002634, 0.00000386194, 0.00007426526, 0.00000001834, 0.00005486540 };

const float         L_ANKLE_ROLL_LINK[] = { 0.0, 0.0, 0.0 };
const float         L_ANKLE_ROLL_COM[] = { 0.16184, 0.02542, 0.00330, -0.03239 };    // _F_O_O_T
const float         L_ANKLE_ROLL_CONSTRAINTS[] = { -0.397880, 0.769001 };
const float         L_ANKLE_ROLL_INERTIA[] = { 0.00026944182, -0.00000569572, 0.00013937948, 0.00064434250, 0.00001874092, 0.00052575675 };



// RIGHT LEG
const float         R_HIP_ROLL_LINK[] = { 0.0, 0.0, 0.0 };
const float         R_HIP_ROLL_COM[] = { 0.13053, -0.01549, -0.00029, -0.00516 };    // _H_I_P
const float         R_HIP_ROLL_CONSTRAINTS[] = { -0.790477, 0.379472 };
const float         R_HIP_ROLL_INERTIA[] = { 0.00002758654, -0.00000001919, -0.00000410822, 0.00009826996, 0.00000000251, 0.00008810332 };

const float         R_HIP_PITCH_LINK[] = { 0.0, 0.0, 0.0 };
const float         R_HIP_PITCH_COM[] = { 0.38976, 0.00139, -0.00225, -0.05374 };    // _T_I_G_H_T
const float         R_HIP_PITCH_CONSTRAINTS[] = { -1.535889, 0.484090 };
const float         R_HIP_PITCH_INERTIA[] = { 0.00163748205, -0.00000083954, 0.00008588301, 0.00159221403, 0.00003917626, 0.00030397824 };

const float         R_KNEE_PITCH_LINK[] = { 0.0, 0.0, -THIGH_LENGTH };
const float         R_KNEE_PITCH_COM[] = { 0.29163, 0.00394, -0.00221, -0.04938 };    // _T_I_B_I_A
const float         R_KNEE_PITCH_CONSTRAINTS[] = { -0.103083, 2.120198 };
const float         R_KNEE_PITCH_INERTIA[] = { 0.00118282956, 0.00000089650, 0.00002799690, 0.00112827851, -0.00003847604, 0.00019145277 };

const float         R_ANKLE_PITCH_LINK[] = { 0.0, 0.0, -TIBIA_LENGTH };
const float         R_ANKLE_PITCH_COM[] = { 0.13415, 0.00045, -0.00030, 0.00684 };    // _A_N_K_L_E
const float         R_ANKLE_PITCH_CONSTRAINTS[] = { -1.186448, 0.932056 };
const float         R_ANKLE_PITCH_INERTIA[] = { 0.00003850813, -0.00000006434, 0.00000387466, 0.00007431082, -0.0000000458, 0.00005491312 };

const float         R_ANKLE_ROLL_LINK[] = { 0.0, 0.0, 0.0 };
const float         R_ANKLE_ROLL_COM[] = { 0.16171, 0.02540, -0.00332, -0.03239 };    // _F_O_O_T
const float         R_ANKLE_ROLL_CONSTRAINTS[] = { -0.768992, 0.397935 };
const float         R_ANKLE_ROLL_INERTIA[] = { 0.00026930201, 0.00000587505, 0.00013913328, 0.00064347385, -0.00001884917, 0.00052503477 };

#endif
