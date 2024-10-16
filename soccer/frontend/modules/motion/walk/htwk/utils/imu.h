#pragma once

#include "point_3d.h"
#include <Eigen/Core>

struct YPR {
    float yaw{}, pitch{}, roll{};

    YPR() = default;
    YPR(const Eigen::Vector3f &g) : yaw(g.z()), pitch(g.y()), roll(g.x()) {}
    YPR(float _yaw, float _pitch, float _roll) : yaw(_yaw), pitch(_pitch), roll(_roll) {}

    YPR operator-(const YPR &o) {
        return {yaw - o.yaw, pitch - o.pitch, roll - o.roll};
    }

    YPR operator+(const YPR &o) {
        return {yaw + o.yaw, pitch + o.pitch, roll + o.roll};
    }
};

struct PitchRoll {
    float pitch{}, roll{};

    PitchRoll() = default;
    PitchRoll(float pitch, float roll) : pitch(pitch), roll(roll) {}
};

struct YawPitch {
    float yaw{}, pitch{};

    YawPitch() = default;
    YawPitch(float yaw, float pitch) : yaw(yaw), pitch(pitch) {}
};

struct IMU {
    YPR gyr;
    point_3d accel;
};
