#pragma once

#include <framework/blackboard/blackboard.h>
#include "../bembelbots/types.h"
#include "../bembelbots/constants.h"
#include "../spl/spl.h"

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
    MAKE_VAR(CompetitionType, competitionType);
    MAKE_VAR(CompetitionPhase, competitionPhase);
    MAKE_VAR(GamePhase, gamePhase);
    MAKE_VAR(GameState, gameState);
    MAKE_VAR(SetPlay, setPlay);

    MAKE_VAR(bool, firstHalf);
    MAKE_VAR(bool, kickoff);
    MAKE_VAR(bool, whistle);

    MAKE_VAR(int, secsRemaining);
    MAKE_VAR(int, secondaryTime);

    // player info
    MAKE_VAR(Penalty, penalty);
    MAKE_VAR(int, secsTillUnpenalised);

    // team info
    MAKE_VAR(int, teamNumber);
    MAKE_VAR(TeamColor, fieldPlayerColor);
    MAKE_VAR(TeamColor, goalkeeperColor);
    MAKE_VAR(int, score);
    MAKE_VAR(int, penaltyShot);
    MAKE_VAR(int, singleShots);
    MAKE_VAR(int, messageBudget);

    // penalties for all players
    using penaltiesArray = std::array<Penalty, NUM_PLAYERS>;
    MAKE_VAR(penaltiesArray, penalties);
    MAKE_VAR(penaltiesArray, opponentPenalties);

    // opponent team's info
    MAKE_VAR(int, opponentTeamNumber);
    MAKE_VAR(TeamColor, opponentTeamColor);
    MAKE_VAR(TeamColor, opponentFieldPlayerColor);
    MAKE_VAR(TeamColor, opponentGoalkeeperColor);
    MAKE_VAR(int, opponentScore);
    MAKE_VAR(int, opponentPenaltyShot);
    MAKE_VAR(int, opponentSingleShots);
    MAKE_VAR(int, opponentMessageBudget);

    MAKE_VAR(TimestampMs, lastPacketTs);
};

std::ostream &operator<<(std::ostream &s, const GamecontrolBlackboard *rhs);


// vim: set ts=4 sw=4 sts=4 expandtab:
