#include <Eigen/Core>

#include "body_chain.h"
#include "lola_names_generated.h"
#define _USE_MATH_DEFINES


namespace bodyConstV5
{
    static const Eigen::Vector3f comNeck{-1e-05, 0.0, -0.02742}, comHead{-0.00112, 0.0, 0.05258};
    static const Eigen::Vector3f comLShoulder{-0.00165, -0.02663, 0.00014}, comLBiceps{0.02455, 0.00563, 0.0033}, comLElbow{-0.02744, 0.0, -0.00014}, comLForearm{0.02556, 0.00281, 0.00076}, comLHand{0.03434, -0.00088, 0.00308};
    static const Eigen::Vector3f comRShoulder{-0.00165, 0.02663, 0.00014}, comRBiceps{0.02455, -0.00563, 0.0033}, comRElbow{-0.02744, 0.0, -0.00014}, comRForearm{0.02556, -0.00281, 0.00076}, comRHand{0.03434, 0.00088, 0.00308};
    static const Eigen::Vector3f comLPelvis{-0.00781, -0.01114, 0.02661}, comLHip{-0.01549, 0.00029, -0.00515}, comLThigh{0.00138, 0.00221, -0.05373}, comLTibia{0.00453, 0.00225, -0.04936}, comLAnkle{0.00045, 0.00029, 0.00685}, comLFoot{0.02542, 0.0033, -0.03239};
    static const Eigen::Vector3f comRPelvis{-0.00781, 0.01114, 0.02661}, comRHip{-0.01549, -0.00029, -0.00515}, comRThigh{0.00138, -0.00221, -0.05373}, comRTibia{0.00453, -0.00225, -0.04936}, comRAnkle{0.00045, -0.00029, 0.00685}, comRFoot{0.02542, -0.0033, -0.03239};
    static const Eigen::Vector3f comTorso{-0.00413, 0.0, 0.04342};

    static const Eigen::Vector3f fromTorsoToHeadYaw{0.0, 0.0, 0.1265};
    static const Eigen::Vector3f fromTorsoToLShoulderPitch{0.0, 0.098, 0.100}, fromLShoulderRollToLElbowYaw{0.105, 0.015, 0.0}, fromLElbowRollToLWristYaw{0.05595, 0.0, 0.0};
    static const Eigen::Vector3f fromTorsoToRShoulderPitch{0.0, -0.098, 0.100}, fromRShoulderRollToRElbowYaw{0.105, -0.015, 0.0}, fromRElbowRollToRWristYaw{0.05595, 0.0, 0.0};
    static const Eigen::Vector3f fromTorsoToRHipYawPitch{0.0, -0.05, -0.085}, fromRHipPitchToRKneePitch{0.0, 0.0, -0.100}, fromRKneePitchToRAnklePitch{0.0, 0.0, -0.1029};
    static const Eigen::Vector3f fromTorsoToLHipYawPitch{0.0, 0.05, -0.085}, fromLHipPitchToLKneePitch{0.0, 0.0, -0.100}, fromLKneePitchToLAnklePitch{0.0, 0.0, -0.1029};
    static const Eigen::Vector3f fromGroundToAnklePitch{0.0, 0.0, 0.04519};
  
    static const Eigen::Vector3f fromRHipYawPitchToTorso{0.0, 0.05, 0.085}, fromRKneePitchToRHipPitch{0.0, 0.0, 0.100}, fromRAnklePitchToRKneePitch{0.0, 0.0, 0.10290};
    static const Eigen::Vector3f fromLHipYawPitchToTorso{0.f, -0.05f, 0.085f}, fromLKneePitchToLHipPitch{0.0, 0.0, 0.100}, fromLAnklePitchToLKneePitch{0.0, 0.0, 0.10290};

    //cam_pose:
    static const Eigen::Vector3f BOTTOM_CAM_OFFSET{0.0f, 0.6929f, 0.0f}, TOP_CAM_OFFSET{0.0f, 0.0209f, 0.0f}, R_FOOT_OFFSET{0.0f, 0.05f, 0.0f};
    static const Eigen::Vector3f ACCELEROMETER_OFFSET{-0.008f, 0.00606f, 0.027f}, GYROMETER_OFFSET{-0.008f, 0.006f, 0.029};
    static const Eigen::Vector4f HEAD_YAW_TO_BOTTOM_CAM{0.05071, 0.0, 0.01774, 1.f}, HEAD_YAW_TO_TOP_CAM{0.05871, 0.0, 0.06364, 1.f}; //todo: find out!

    // masses:
    static constexpr float massRobot = 5.305350006;
    static constexpr float massShoulder = 0.09304;
    static constexpr float massBiceps = 0.15777;
    static constexpr float massElbow = 0.06483;
    static constexpr float massForeArm = 0.07761;
    static constexpr float massHand = 0.18533;
    static constexpr float massArm = massShoulder + massBiceps + massElbow + massForeArm + massHand;
    static constexpr float massPelvis = 0.06981;
    static constexpr float massHip = 0.14053;
    static constexpr float massThigh = 0.38968;
    static constexpr float massTibia = 0.30142;
    static constexpr float massAnkle = 0.13416;
    static constexpr float massFoot = 0.17184;
    static constexpr float massLeg = massPelvis + massHip + massThigh + massTibia + massAnkle + massFoot;
    static constexpr float massHead = 0.60533;
    static constexpr float massNeck = 0.07842;
    static constexpr float massNeckHead = massHead + massNeck;
    static constexpr float massTorso = 1.0496;

    // Offset for DH-Parameter (_M -> in meter, because of collision with definitionsBody.h, where similar names are used for millimeter constants)
    static const float HIP_OFFSET_Z_M = 0.085f;
    static const float HIP_OFFSET_Y_M = 0.05f;
    static const float THIGH_LENGTH_M = 0.1f;
    static const float TIBIA_LENGTH_M = 0.1029f;
    static const float FOOT_HEIGHT_M = 0.04519f;

    // Denavit Hartenberg Parameter
    // left leg:
    static const DHParameter DH_L_HIP           {0.f, -(HIP_OFFSET_Z_M-HIP_OFFSET_Y_M), 0.f, -3.f*M_PI_4, false};
    static const DHParameter DH_L_HIP_YAW_PITCH {-M_PI_2, sqrtf(HIP_OFFSET_Y_M * HIP_OFFSET_Y_M * 2.f), 0.f, -M_PI_2, true};
    static const DHParameter DH_L_HIP_ROLL      {-3.f*M_PI_4, 0.f, 0.f, -M_PI_2, true};
    static const DHParameter DH_L_HIP_PITCH     {0.f, 0.f, THIGH_LENGTH_M, 0.f, true};
    static const DHParameter DH_L_KNEE_PITCH    {0.f, 0.f, TIBIA_LENGTH_M, 0.f, true};
    static const DHParameter DH_L_ANKLE_PITCH   {0.f, 0.f, 0.f, M_PI_2, true};
    static const DHParameter DH_L_ANKLE_ROLL    {M_PI_2, 0.f, 0.f, -M_PI_2, true};

    // right leg:
    static const DHParameter DH_R_HIP           {0.f, -(HIP_OFFSET_Z_M-HIP_OFFSET_Y_M), 0.f, -M_PI_4, false};
    static const DHParameter DH_R_HIP_YAW_PITCH {-M_PI_2, -sqrtf(HIP_OFFSET_Y_M * HIP_OFFSET_Y_M * 2.f), 0.f, -M_PI_2, true};
    static const DHParameter DH_R_HIP_ROLL      {3.f*M_PI_4, 0.f, 0.f, -M_PI_2, true};
    static const DHParameter DH_R_HIP_PITCH     {0.f, 0.f, THIGH_LENGTH_M, 0.f, true};
    static const DHParameter DH_R_KNEE_PITCH    {0.f, 0.f, TIBIA_LENGTH_M, 0.f, true};
    static const DHParameter DH_R_ANKLE_PITCH   {0.f, 0.f, 0.f, M_PI_2, true};
    static const DHParameter DH_R_ANKLE_ROLL    {M_PI_2, 0.f, 0.f, -M_PI_2, true};

    static const DHParameter DH_FOOT         {-M_PI_2, -FOOT_HEIGHT_M, 0.f, 0.f, false};
    // head:
    static const DHParameter DH_CHEST {0.f, fromTorsoToHeadYaw[2], 0.f, 0.f, false};
    static const DHParameter DH_NECK {0.f, 0.f, 0.f, -M_PI_2, true};
    static const DHParameter DH_HEAD {0.f, 0.f, 0.f, M_PI_2, true};

    BodyChain lLeg{ {DH_L_HIP, DH_L_HIP_YAW_PITCH, DH_L_HIP_ROLL, DH_L_HIP_PITCH, DH_L_KNEE_PITCH, DH_L_ANKLE_PITCH, DH_L_ANKLE_ROLL, DH_FOOT}, 
                    {0, int(JointNames::LHipYawPitch), int(JointNames::LHipRoll), int(JointNames::LHipPitch), int(JointNames::LKneePitch), int(JointNames::LAnklePitch), int(JointNames::LAnkleRoll), 0} };

    BodyChain rLeg{ {DH_R_HIP, DH_R_HIP_YAW_PITCH, DH_R_HIP_ROLL, DH_R_HIP_PITCH, DH_R_KNEE_PITCH, DH_R_ANKLE_PITCH, DH_R_ANKLE_ROLL, DH_FOOT}, 
                    {0, int(JointNames::LHipYawPitch), int(JointNames::RHipRoll), int(JointNames::RHipPitch), int(JointNames::RKneePitch), int(JointNames::RAnklePitch), int(JointNames::RAnkleRoll), 0} };

    BodyChain head{ {DH_CHEST, DH_NECK, DH_HEAD},
                    {0, int(JointNames::HeadYaw), int(JointNames::HeadPitch)} };

}
