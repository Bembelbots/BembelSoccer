#pragma once

#include <core/util/constants.h>
#include <core/util/mathtoolbox.h>
#include <shared/common/types.h>
#include <worldmodel/definitions.h>

#include <array>

class PlayingField;
class ReactiveWalkBlackboard;

namespace stab {

struct Input {

    TimestampMs timeMs = 0; //< current time in ms
    GameState gamestate = GameState::INITIAL;
    SetPlay setPlay = SetPlay::NONE;
    bool hasKickoff = true;
    BotId botid = BotId::One;
    PlayingField *field = nullptr;
    Coord ballPosWcs;
    int ballAge;
    float ballSize;
    DirectedCoord myPos;
    float headYaw;
    float robotSize;

    std::vector<DirectedCoord> detectedBots;
    std::array<Robot, NUM_PLAYERS> myTeam;
    std::array<Ball, NUM_PLAYERS> seenBallsOfMyTeam;

    int activeBots;

    bool isMoving;
};

} // namespace stab

// vim: set ts=4 sw=4 sts=4 expandtab:
