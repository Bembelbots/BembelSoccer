//
// Created by felix on 21.04.18.
//

#include "body_interface.h"

const std::string BodyInterface::blackboardName = "BodyInterface";

BodyInterface::BodyInterface()
  : Blackboard(blackboardName) {
    INIT_VAR(mcs, DirectedCoord(), "");
    INIT_SWITCH(resetMcs, false, "");

    INIT_VAR(accel, Eigen::Vector3f(), "accelerometer (X,Y,Z)");
    INIT_VAR(gyro, Eigen::Vector3f(), "gyroscope (X,Y,Z)");
    INIT_VAR(bodyAngles, Eigen::Vector3f(), "torso angles (X,Y,Z / roll, pitch, yaw)");

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
    if (DEBUG_ON(resetMcs)) {
        DEBUG_ON(resetMcs) = false;
        odoResetPos = body.odometry;
    }
    mcs = body.odometry - odoResetPos;

    accel = body.accel;
    gyro = body.gyro;
    bodyAngles = body.bodyAngles;

    fallenSide = enumToStr(body.fallenSide);

    lFSRTotal = body.sensors[lFSRTotalSensor];
    rFSRTotal = body.sensors[rFSRTotalSensor];

    lBumper = body.sensors[lBumperLeftSensor] + 2 * body.sensors[lBumperRightSensor];
    rBumper = body.sensors[rBumperLeftSensor] + 2 * body.sensors[rBumperRightSensor];

    maxTempHead = getMaxSensor(body, {headYawTemperatureSensor, headPitchTemperatureSensor});

    maxTempArmR = getMaxSensor(body,
            {rShoulderPitchTemperatureSensor,
                    rShoulderRollTemperatureSensor,
                    rElbowYawTemperatureSensor,
                    rElbowRollTemperatureSensor,
                    rWristYawTemperatureSensor});
    maxTempArmL = getMaxSensor(body,
            {lShoulderPitchTemperatureSensor,
                    lShoulderRollTemperatureSensor,
                    lElbowYawTemperatureSensor,
                    lElbowRollTemperatureSensor,
                    lWristYawTemperatureSensor});

    maxTempLegR = getMaxSensor(body,
            {rHipRollTemperatureSensor,
                    rKneePitchTemperatureSensor,
                    rAnklePitchTemperatureSensor,
                    rAnkleRollTemperatureSensor});
    maxTempLegL = getMaxSensor(body,
            {lHipYawPitchTemperatureSensor,
                    lHipRollTemperatureSensor,
                    lKneePitchTemperatureSensor,
                    lAnklePitchTemperatureSensor,
                    lAnkleRollTemperatureSensor});

    maxCurrentHead = getMaxSensor(body, {headYawCurrentSensor, headPitchCurrentSensor});

    maxCurrentArmR = getMaxSensor(body,
            {rShoulderPitchCurrentSensor,
                    rShoulderRollCurrentSensor,
                    rElbowYawCurrentSensor,
                    rElbowRollCurrentSensor,
                    rWristYawCurrentSensor});
    maxCurrentArmL = getMaxSensor(body,
            {lShoulderPitchCurrentSensor,
                    lShoulderRollCurrentSensor,
                    lElbowYawCurrentSensor,
                    lElbowRollCurrentSensor,
                    lWristYawCurrentSensor});

    maxCurrentLegR = getMaxSensor(
            body, {rHipRollCurrentSensor, rKneePitchCurrentSensor, rAnklePitchCurrentSensor, rAnkleRollCurrentSensor});
    maxCurrentLegL = getMaxSensor(body,
            {lHipYawPitchCurrentSensor,
                    lHipRollCurrentSensor,
                    lKneePitchCurrentSensor,
                    lAnklePitchCurrentSensor,
                    lAnkleRollCurrentSensor});

    batteryCharge = body.sensors[batteryChargeSensor];
    batteryCurrent = body.sensors[batteryCurrentSensor];
    batteryTemperature = body.sensors[batteryTemperatureSensor];

    lSonar = body.sensors[lUsSensor];
    rSonar = body.sensors[rUsSensor];
}

float BodyInterface::getMaxSensor(const BodyState &body, const std::vector<int> &sensors) {
    float max{-1000};
    for (const int &i : sensors)
        if (body.sensors[i] > max)
            max = body.sensors[i];
    return max;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
