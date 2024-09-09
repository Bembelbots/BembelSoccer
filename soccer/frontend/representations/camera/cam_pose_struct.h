#pragma once
#include <Eigen/Core>

struct CamPose
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Eigen::Vector3f v, r;

    CamPose() = default;

    bool operator==(const CamPose& other) const {
        return v == other.v && r == other.r;
    }

    bool operator!=(const CamPose& other) const {
        return !(*this == other);
    }
};
