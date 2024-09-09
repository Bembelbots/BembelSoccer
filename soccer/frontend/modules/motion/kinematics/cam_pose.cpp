#include "cam_pose.h"

#include <framework/logger/logger.h>
#include <framework/math/rotation_matrices.h>
#include "body_const_v5.h"

// avoid warning about unused function
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// for debugging/unittests only:
static float round(float f, uint d = 6) {
    int m = pow(10, d);
    return std::round(f * m) / m;
}

// for debugging/unittests only:
static Eigen::Matrix3f round(const Eigen::Matrix3f &M, uint d = 6) {
    int m = pow(10, d);
    Eigen::Matrix3f T = M * m;

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            T(r, c) = std::round(T(r, c));
        }
    }
    return T / m;
}

#pragma GCC diagnostic pop

Eigen::Matrix4f CameraPose::getHeadTransformation(bbipc::Sensors &s, const Eigen::Vector3f &a, float balancingFoot) {
    using namespace bodyConstV5;

    Eigen::Matrix4f bodyTransformation, ahrsTransformation;
    Eigen::Vector4f torsoTranslation;

    if (balancingFoot > BALANCING_FOOT_TRESHOLD) { // left foot
        bodyTransformation = RotMat::translate(R_FOOT_OFFSET) * lLeg.transformation(s, -1);
    } else if (balancingFoot < -BALANCING_FOOT_TRESHOLD) { // right foot
        bodyTransformation = RotMat::translate(-R_FOOT_OFFSET) * rLeg.transformation(s, -1);

    } else { // both feet
        Eigen::Matrix4f rLi = rLeg.transformation(s, -1);
        float rFootToLFoot = (rLi * lLeg.transformation(s, 1))(1, 3);
        bodyTransformation = RotMat::translate(0.f, -0.5f * rFootToLFoot, 0.f) * rLi;
    }

    torsoTranslation =
            RotMat::homogenous(bodyTransformation.block<3, 3>(0, 0).transpose() * bodyTransformation.block<3, 1>(0, 3));

    ahrsTransformation = RotMat::rotateRPY(a(0), a(1), 0.f);

    ahrsTransformation.block<4, 1>(0, 3) = ahrsTransformation * torsoTranslation;

    return ahrsTransformation * head.transformation(s);
}

Eigen::Vector3f CameraPose::getAnglesFromMatrix(const Eigen::Matrix4f &T) {
    Eigen::Vector3f a;

    a[0] = atan2(T(2, 1), T(2, 2));
    a[1] = asinf(-T(2, 0));
    a[2] = atan2(T(1, 0), T(0, 0));

    return a;
}

std::array<CamPose, 2> CameraPose::getCameraPose(const Eigen::Matrix4f &T) {
    using namespace bodyConstV5;

    std::array<CamPose, 2> camPoses;

    Eigen::Vector3f headRot;
    Eigen::Vector4f n4{0.f, 0.f, 0.f, 1.f};

    headRot = getAnglesFromMatrix(T);

    // --- add camera offset relative to x-axis (manufacturers documentation)
    camPoses[0].r = headRot + BOTTOM_CAM_OFFSET;
    camPoses[1].r = headRot + TOP_CAM_OFFSET;

    // --- using headRot, add last piece of trajectory
    camPoses[0].v = (T * HEAD_YAW_TO_BOTTOM_CAM).block<3, 1>(0, 0);
    camPoses[1].v = (T * HEAD_YAW_TO_TOP_CAM).block<3, 1>(0, 0);

    return camPoses;
}

std::array<CamPose, 2> CameraPose::getPose(
        bbipc::Sensors &sensors, Eigen::Vector3f bodyAngles, const float balancingFoot) {
    return getCameraPose(getHeadTransformation(sensors, bodyAngles, balancingFoot));
}
