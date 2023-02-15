#pragma once

#include <framework/rt/message_utils.h>
#include "../spl/spl.h"

struct GamecontrolMessage {
    TeamColor teamColor;
    GameState gameState;
    GamePhase gamePhase;
    SetPlay setPlay;
    bool kickoff;
    bool unstiff;
    Penalty penalty;
};
