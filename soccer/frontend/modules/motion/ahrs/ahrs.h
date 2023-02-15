#pragma once
#include <vector>
#include <Eigen/Core>

#include <framework/math/angle.h>
#include <representations/bembelbots/types.h>

class AHRSProvider {
public:
    AHRSProvider();
    virtual ~AHRSProvider();

    void update(Eigen::Vector3f g, Eigen::Vector3f a);

    Angle getPitch();
    Angle getYaw();
    Angle getRoll();

    Eigen::Vector3f getAcceleration();

    Eigen::Vector3f getEuler();

private:
    Eigen::Vector3f euler;  // pitch, yaw, roll
    Eigen::Vector3f acc;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
