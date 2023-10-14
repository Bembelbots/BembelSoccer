#pragma once
#include <framework/rt/message_utils.h>
#include <framework/math/directed_coord.h>
#include <roles_generated.h>

struct TeamcommCommand {};

struct TeamcommDebugInfo {
    DirectedCoord walktarget_pos;
    Coord dribble_target;
    bbapi::RobotRole role;
};

RT_REGISTER_COMMAND(TeamcommDebugInfo, TeamcommCommand);
