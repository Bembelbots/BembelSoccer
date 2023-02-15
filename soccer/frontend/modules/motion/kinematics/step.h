#pragma once

#include <array>
#include <libbembelbots/bembelbots.h>

struct Step{
    std::array<float, lbbNumOfPositionActuatorIds> angles{};
    std::array<float, lbbNumOfPositionActuatorIds> stiffnesses{};
    bool requestAngles;
};
