#pragma once
#include <representations/camera/cam_pose_struct.h>
#include <framework/util/clock.h>
#include <libbembelbots/bembelbots.h>

struct NaoState {
    TimestampMs timestamp_ms;
    int64_t lola_timestamp;
    bool connected;
    camPose bCamPose;
    camPose tCamPose;
    BBActuatorData *actuatorData;
    const BBSensorData *sensorData;
};
