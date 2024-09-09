#pragma once

#include "dh_parameter.h"
#include <framework/logger/logger.h>
#include <Eigen/Core>
#include <vector>
#include <framework/joints/joints.hpp>


class BodyChain
{
private:
    const std::vector<DHParameter> dh;
    const std::vector<int> j;

    Eigen::Matrix4f DHTransform(const bbipc::Sensors &s, int i);

public:
    BodyChain(const std::vector<DHParameter> &dh, const std::vector<int> &j)
        : dh(dh)
        , j(j) {
    }

    Eigen::Matrix4f transformation(const bbipc::Sensors &sensors, int initial, int final);
    Eigen::Matrix4f transformation(const bbipc::Sensors &sensors, int dir = 1);
};
