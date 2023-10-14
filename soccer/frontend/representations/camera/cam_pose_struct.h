#pragma once
#include <Eigen/Core>

struct camPose
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Eigen::Vector3f v, r;

    camPose() = default;

    bool operator==(const camPose& other) const {
        return v == other.v && r == other.r;
    }

    bool operator!=(const camPose& other) const {
        return !(*this == other);
    }
};
