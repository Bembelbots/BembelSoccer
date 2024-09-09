#include "gamecontrol.h"
#include <framework/util/assert.h>

using namespace bbapi;

GamecontrolBlackboard::GamecontrolBlackboard() :
    SnapshotableBlackboard("Gamecontrol") {
    setup();
}

void GamecontrolBlackboard::setup() {
    INIT_VAR_RW(unstiff, false,
                 "unstiff robot (sit down and don't do anything)");

    INIT_VAR(competitionType,static_cast<int>(CompetitionType::NORMAL),
                 "competition type (normal, mixed team, general penalty kick)");
    INIT_VAR(competitionPhase,static_cast<int>(CompetitionPhase::ROUNDROBIN),
                 "competition phase (normal, penalty shoot, overtime, timeout)");

    INIT_VAR(gamePhase,static_cast<int>(GamePhase::NORMAL),
                 "game phase (normal, penalty shoutout, overtime, timeout)");
    INIT_VAR_RW(gameState,static_cast<int>(GameState::INITIAL),
                 "game state (initial, ready, set, play, finish)");
    
    INIT_VAR(setPlay,static_cast<int>(SetPlay::NONE),
                 "set play (none, free kick, pushing free kick)");

    INIT_VAR(firstHalf, true, "true in first half of the game");
    INIT_VAR(kickoff, false, "true of own team has kick-off");

    INIT_VAR(secsRemaining, 600, "remaining playing time");
    INIT_VAR(secondaryTime, 0,
                "number of seconds shown as secondary time (remaining ready, until free ball, etc)");

    INIT_VAR_RW(penalty,static_cast<int>(Penalty::NONE), "current penalty");
    INIT_VAR(secsTillUnpenalised, 0, "time remaining for penalty");

    INIT_VAR(lastPacketTs, -1, "timestamp of last valid gamecontroller packet");
}

std::ostream &operator<<(std::ostream &s, const GamecontrolBlackboard *rhs) {
    s << "current gamecontrol blackboard content:\n";
    s << "  competitionType:            " << int(rhs->competitionType)          << "\n";
    s << "  competitionPhase:           " << int(rhs->competitionPhase)         << "\n";
    s << "  gameState:                  " << int(rhs->gameState)                << "\n";
    s << "  gamePhase:                  " << int(rhs->gamePhase)                << "\n";
    s << "  firstHalf:                  " << rhs->firstHalf                     << "\n";
    s << "  firstHalf:                  " << rhs->firstHalf                     << "\n";
    s << "  firstHalf:                  " << rhs->firstHalf                     << "\n";
    s << "  firstHalf:                  " << rhs->firstHalf                     << "\n";
    s << "  firstHalf:                  " << rhs->firstHalf                     << "\n";
    s << "  firstHalf:                  " << rhs->firstHalf                     << "\n";
    s << "  kickoff:                    " << rhs->kickoff                       << "\n";
    s << "  secsRemaining:              " << rhs->secsRemaining                 << "\n";
    s << "  secondaryTime:              " << rhs->secondaryTime                 << "\n";
    s << "  penalty:                    " << int(rhs->penalty)                  << "\n";
    s << "  secsTillUnpenalised:        " << rhs->secsTillUnpenalised           << "\n";
    return s;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
