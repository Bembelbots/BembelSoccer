#pragma once
#include <array>
#include "bembelbots.h"

static float sit(const float *sensors, float *actuators) {
    static const std::array<float, lbbNumOfPositionActuatorIds> sitDownAngles{
        0.f,
        0.f,

        1.64f,
        0.f,
        -1.17f,
        -1.25f,
        -1.57f,
        0.f,

        1.64f,
        0.f,
        1.17f,
        1.25f,
        1.57f,
        0.f,

        0.f,

        0.f,
        -0.87f,
        2.11f,
        -1.18f,
        0.f,

        0.f,
        -0.87f,
        2.11f,
        -1.18f,
        0.f
    };
    static std::array<float, lbbNumOfPositionActuatorIds> startAngles;
    static float phase{0};

    if (phase == 0)
        for (int i = 0; i < lbbNumOfPositionActuatorIds; ++i)
            startAngles[i] = sensors[i * 3];
        
    for (int i = 0; i < lbbNumOfPositionActuatorIds; ++i)
        actuators[headYawHardnessActuator + i] = 0.6f;

    phase = std::min(phase + 0.01f, 1.0f);

    for (int i = 0; i < lbbNumOfPositionActuatorIds; ++i)
        actuators[i] =  phase * sitDownAngles[i] + (1.0f - phase) * startAngles[i];

    if (phase >= 1.0f) {
        for(int i = 0; i < lbbNumOfPositionActuatorIds; ++i)
            actuators[headYawHardnessActuator + i] = -1;

        // set actuators to sensor angles to relax  any joints,
        // that could not reach their target
        for (int i = 0; i < lbbNumOfPositionActuatorIds; ++i)
            actuators[i] = sensors[i * 3];
        
        actuators[lHipPitchHardnessActuator] = actuators[rHipPitchHardnessActuator] = 0.15f;
        //actuators[lHipYawPitchHardnessActuator] = 0.05f;

        phase = 0;
        return true;
    }

    return false;
}


// vim: set ts=4 sw=4 sts=4 expandtab:
