#pragma once
#include <framework/serialize/serializer.h>
#include <Eigen/Dense>

SERIALIZE(Eigen::Vector3f, {
    std::string _key = key;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) <<
         "[ " << value(0) << ", " << value(1) << ", " << value(2) << " ]";
    std::string _value = ss.str();
    return {_key, _value};
});