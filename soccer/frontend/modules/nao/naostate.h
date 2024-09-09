#pragma once

#include "ipc_actuator_message_generated.h"
#include "ipc_sensor_message_generated.h"
#include <framework/util/clock.h>

#include <representations/camera/cam_pose_struct.h>
#include <representations/flatbuffers/types/sensors.h>
#include <representations/flatbuffers/types/actuators.h>

struct NaoState {
    TimestampMs timestamp_ms;
    int64_t lola_timestamp;
    bool connected;
    CamPose bCamPose;
    CamPose tCamPose;
    bbapi::BembelIpcActuatorMessageT *actuatorData;
    const bbapi::BembelIpcSensorMessageT *sensorData;
};
