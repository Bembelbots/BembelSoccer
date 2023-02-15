#pragma once
#include "../spl/RoboCupGameControlData.h"
#include "../worldmodel/definitions.h"

static constexpr int numPlayers{MAX_NUM_PLAYERS};

struct TeamMessage {
    Robot robot;
    Ball ball;
    TimestampMs timestamp;
};
