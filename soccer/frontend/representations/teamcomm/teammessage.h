#pragma once

#include <vector>
#include <optional>

#include "../spl/RoboCupGameControlData.h"
#include "../worldmodel/definitions.h"

static constexpr int numPlayers{MAX_NUM_PLAYERS};

struct TeamMessage {
    Robot robot;
    std::optional<Ball> ball;
    std::optional<Ball> team_ball;
    bool nearest_to_team_ball;
    std::optional<DirectedCoord> walktarget;
    std::optional<Coord> goaltarget;
    std::vector<Coord> obstacles;
    TimestampMs timestamp;
    bool refereeGestureUp;
};
