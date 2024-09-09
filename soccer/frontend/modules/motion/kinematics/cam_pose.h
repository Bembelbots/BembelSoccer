#pragma once

#include <array>
#include <Eigen/Core>
#include <framework/joints/joints.hpp>
#include <representations/camera/cam_pose_struct.h>

class CameraPose {
private:
    static constexpr float BALANCING_FOOT_TRESHOLD = 0.5f;

    Eigen::Matrix4f getHeadTransformation(bbipc::Sensors &s, const Eigen::Vector3f &a, float balancingFoot);

    static Eigen::Vector3f getAnglesFromMatrix(const Eigen::Matrix4f &T);

    std::array<CamPose, 2> getCameraPose(const Eigen::Matrix4f &T);

public:
    std::array<CamPose, 2> getPose(bbipc::Sensors &sensors, Eigen::Vector3f bodyAngles, const float balancingFoot);
};
