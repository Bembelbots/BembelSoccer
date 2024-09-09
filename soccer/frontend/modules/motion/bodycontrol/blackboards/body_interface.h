//
// Created by felix on 21.04.18.
//
#pragma once

#include <Eigen/Core>
#include <framework/blackboard/blackboard.h>
#include <representations/motion/body_state.h>
#include <representations/serialize/eigen.h>

class BodyInterface : public Blackboard {

public:
    static const std::string blackboardName;

    BodyInterface();

    MAKE_VAR(DirectedCoord, mcs);
    MAKE_VAR(bool, resetMcs);

    MAKE_VAR(Eigen::Vector3f, accel);
    MAKE_VAR(Eigen::Vector3f, gyro);
    MAKE_VAR(Eigen::Vector3f, bodyAngles);

    MAKE_VAR(std::string, fallenSide);

    MAKE_VAR(float, lFSRTotal);
    MAKE_VAR(float, rFSRTotal);

    MAKE_VAR(float, lBumper);
    MAKE_VAR(float, rBumper);

    MAKE_VAR(float, maxTempHead);
    MAKE_VAR(float, maxTempArmR);
    MAKE_VAR(float, maxTempArmL);
    MAKE_VAR(float, maxTempLegR);
    MAKE_VAR(float, maxTempLegL);

    MAKE_VAR(float, maxCurrentHead);
    MAKE_VAR(float, maxCurrentArmR);
    MAKE_VAR(float, maxCurrentArmL);
    MAKE_VAR(float, maxCurrentLegR);
    MAKE_VAR(float, maxCurrentLegL);

    MAKE_VAR(float, batteryCharge);
    MAKE_VAR(float, batteryCurrent);
    MAKE_VAR(float, batteryTemperature);

    MAKE_VAR(float, lSonar);
    MAKE_VAR(float, rSonar);

    void update(const BodyState &);

private:
    DirectedCoord odoResetPos;

    float getMaxSensor(const bbipc::JointArray &ja, const std::vector<bbapi::JointNames> &sensors);
};

// vim: set ts=4 sw=4 sts=4 expandtab:
