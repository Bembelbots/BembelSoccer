#pragma once
#include <framework/rt/message_utils.h>
#include <framework/math/directed_coord.h>

struct TeamcommCommand {};

struct ReactiveWalkDebug {
    DirectedCoord walktarget_pos;
    Coord goaltarget;
};

RT_REGISTER_COMMAND(ReactiveWalkDebug, TeamcommCommand);
