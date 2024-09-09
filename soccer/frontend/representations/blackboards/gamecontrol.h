#pragma once

#include <framework/blackboard/blackboard.h>
#include "../bembelbots/types.h"
#include "../bembelbots/constants.h"
#include <gc_enums_generated.h>

#include <array>
#include <memory>

class GamecontrolBlackboard : public SnapshotableBlackboard {
public:
    using SnapshotableBlackboard::SnapshotableBlackboard;
    explicit GamecontrolBlackboard();

    void setup();
 
    // special unstiffed state
    MAKE_VAR(bool, unstiff);

    // game info
    MAKE_VAR(int, competitionType);
    MAKE_VAR(int, competitionPhase);
    MAKE_VAR(int, gamePhase);
    MAKE_VAR(int, gameState);
    MAKE_VAR(int, setPlay);

    MAKE_VAR(bool, firstHalf);
    MAKE_VAR(bool, kickoff);
    MAKE_VAR(bool, whistle);

    MAKE_VAR(int, secsRemaining);
    MAKE_VAR(int, secondaryTime);

    // player info
    MAKE_VAR(int, penalty);
    MAKE_VAR(int, secsTillUnpenalised);

    MAKE_VAR(TimestampMs, lastPacketTs);
};

std::ostream &operator<<(std::ostream &s, const GamecontrolBlackboard *rhs);


// vim: set ts=4 sw=4 sts=4 expandtab:
