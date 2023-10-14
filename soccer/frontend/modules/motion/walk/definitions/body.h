#pragma once
#include "../htwk/ankle_balancer.h"

struct PitchError {
    float pitch;
    float gyro;
};

struct Body {
    float gyro_deadband = 0;
    float body_pitch_deadband_min = 0.10;
    float body_pitch_deadband_max = 0.25;

    PitchError pitch_error;

    float body_pitch;
    float body_roll;
    
    float gyro_pitch;
    float gyro_yaw;

    Body(float body_pitch, float body_roll, float gyro_pitch, float gyro_yaw)
        : body_pitch(body_pitch), body_roll(body_roll), gyro_pitch(gyro_pitch), gyro_yaw(gyro_yaw) {

    }

    void proceed();
};
