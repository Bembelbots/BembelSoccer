//
// Created by felix on 21.04.18.
//

#include <algorithm> 

#include "body_interface.h"
#include "lola_names_generated.h"
#include "representations/flatbuffers/types/sensors.h"

const std::string BodyInterface::blackboardName = "BodyInterface";

BodyInterface::BodyInterface()
  : Blackboard(blackboardName) {
    INIT_VAR(mcs, DirectedCoord(), "");
    INIT_SWITCH(resetMcs, false, "");

    INIT_VAR(accel, Eigen::Vector3f(0,0,0), "accelerometer (X,Y,Z)");
    INIT_VAR(gyro, Eigen::Vector3f(0,0,0), "gyroscope (X,Y,Z)");
    INIT_VAR(bodyAngles, Eigen::Vector3f(0,0,0), "torso angles (X,Y,Z / roll, pitch, yaw)");

    INIT_VAR(fallenSide, "", "");

    INIT_VAR(lFSRTotal, 0, "combined FSR sensor values (left)");
    INIT_VAR(rFSRTotal, 0, "combined FSR sensor values (right)");

    INIT_VAR(lBumper, 0, "combined foot bumper state (left)");
    INIT_VAR(rBumper, 0, "combined foot bumper state (right)");

    INIT_VAR(maxTempHead, -1, "Max Temperature (Head)");
    INIT_VAR(maxTempArmR, -1, "Max Temperature (Right Arm)");
    INIT_VAR(maxTempArmL, -1, "Max Temperature (Left Arm)");
    INIT_VAR(maxTempLegR, -1, "Max Temperature (Right Leg)");
    INIT_VAR(maxTempLegL, -1, "Max Temperature (Left Leg)");

    INIT_VAR(maxCurrentHead, -1, "Max Current (Head)");
    INIT_VAR(maxCurrentArmR, -1, "Max Current (Right Arm)");
    INIT_VAR(maxCurrentArmL, -1, "Max Current (Left Arm)");
    INIT_VAR(maxCurrentLegR, -1, "Max Current (Right Leg)");
    INIT_VAR(maxCurrentLegL, -1, "Max Current (Left Leg)");

    INIT_VAR(batteryCharge, -1, "Battery charge remaining");
    INIT_VAR(batteryCurrent, -1, "Current drwan from battery");
    INIT_VAR(batteryTemperature, -1, "Battery temperature");
    
    INIT_VAR(lSonar, 0, "Sonar sensor value (left)");
    INIT_VAR(rSonar, 0, "Sonar sensor value (right)");
}

void BodyInterface::update(const BodyState &body) {
    const auto &s{body.sensors};

    if (DEBUG_ON(resetMcs)) {
        DEBUG_ON(resetMcs) = false;
        odoResetPos = body.odometry;
    }
    mcs = body.odometry - odoResetPos;

    accel = body.sensors.imu.accelerometer;
    gyro = body.sensors.imu.gyroscope;
    bodyAngles = body.bodyAngles;

    fallenSide = enumToStr(body.fallenSide);

    lFSRTotal = 0;
    for (float f: s.fsr.left.arr)
        lFSRTotal += f;

    rFSRTotal = 0;
    for (float f: s.fsr.right.arr)
        rFSRTotal += f;

    const auto &f{s.touch.feet};
    lBumper = f.left.bumper_left + 2 * f.left.bumper_right;
    rBumper = f.right.bumper_left + 2 * f.right.bumper_right;

    const auto &temp{s.joints.temperature};
    maxTempHead = getMaxSensor(temp, {JointNames::HeadYaw, JointNames::HeadPitch});

    maxTempArmR = getMaxSensor(temp,
            {JointNames::RShoulderPitch,
                JointNames::RShoulderRoll,
                JointNames::RElbowYaw,
                JointNames::RElbowRoll,
                JointNames::RWristYaw});

    maxTempArmL = getMaxSensor(temp,
            {JointNames::LShoulderPitch,
                JointNames::LShoulderRoll,
                JointNames::LElbowYaw,
                JointNames::LElbowRoll,
                JointNames::LWristYaw});

    maxTempLegR = getMaxSensor(temp,
            {JointNames::RHipRoll,
                JointNames::RKneePitch,
                JointNames::RAnklePitch,
                JointNames::RAnkleRoll});

    maxTempLegL = getMaxSensor(temp,
            {JointNames::LHipYawPitch,
                JointNames::LHipRoll,
                JointNames::LKneePitch,
                JointNames::LAnklePitch,
                JointNames::LAnkleRoll});

    const auto &curr{s.joints.current};
    maxCurrentHead = getMaxSensor(curr, {JointNames::HeadPitch, JointNames::HeadYaw});

    maxCurrentArmR = getMaxSensor(curr,
            {JointNames::RShoulderPitch,
                JointNames::RShoulderRoll,
                JointNames::RElbowYaw,
                JointNames::RElbowRoll,
                JointNames::RWristYaw});

    maxCurrentArmL = getMaxSensor(curr,
         {JointNames::LShoulderPitch,
                JointNames::LShoulderRoll,
                JointNames::LElbowYaw,
                JointNames::LElbowRoll,
                JointNames::LWristYaw});

    maxCurrentLegR = getMaxSensor(curr,
            {JointNames::RHipRoll,
                JointNames::RKneePitch,
                JointNames::RAnklePitch,
                JointNames::RAnkleRoll});

    maxCurrentLegL = getMaxSensor(curr,
            {JointNames::LHipYawPitch,
                JointNames::LHipRoll,
                JointNames::LKneePitch,
                JointNames::LAnklePitch,
                JointNames::LAnkleRoll});

    batteryCharge = s.battery.charge;
    batteryCurrent = s.battery.current;
    batteryTemperature = s.battery.temperature;

    lSonar = s.sonar.left;
    rSonar = s.sonar.right;
}

float BodyInterface::getMaxSensor(const bbipc::JointArray &ja, const std::vector<bbapi::JointNames> &sensors) {
    float max{-1000};
    for (const bbapi::JointNames &i : sensors)
        max = std::max(max, ja[int(i)]);
    return max;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
