#include "gamecontrol.h"
#include <framework/util/assert.h>

GamecontrolBlackboard::GamecontrolBlackboard() :
    SnapshotableBlackboard("Gamecontrol") {
    setup();
}

void GamecontrolBlackboard::setup() {
    gameState = GameState::INITIAL;

    INIT_VAR_RW(unstiff, false,
                 "unstiff robot (sit down and don't do anything)");

    INIT_ENUM_RW(competitionType, CompetitionType::NORMAL,
                 "competition type (normal, mixed team, general penalty kick)");
    INIT_ENUM_RW(competitionPhase, CompetitionPhase::ROUNDROBIN,
                 "competition phase (normal, penalty shoot, overtime, timeout)");

    INIT_ENUM_RW(gamePhase, GamePhase::NORMAL,
                 "game phase (normal, penalty shoutout, overtime, timeout)");
    INIT_ENUM_RW(gameState, GameState::INITIAL,
                 "game state (initial, ready, set, play, finish)");
    
    INIT_ENUM_RW(setPlay, SetPlay::NONE,
                 "set play (none, free kick, pushing free kick)");

    INIT_VAR_RW(firstHalf, true, "true in first half of the game");
    INIT_VAR_RW(kickoff, false, "true of own team has kick-off");

    INIT_VAR_RW(secsRemaining, 600, "remaining playing time");
    INIT_VAR_RW(secondaryTime, 0,
                "number of seconds shown as secondary time (remaining ready, until free ball, etc)");

    INIT_ENUM_RW(penalty, Penalty::NONE, "current penalty");
    INIT_VAR_RW(secsTillUnpenalised, 0, "time remaining for penalty");

    INIT_VAR_RW(teamNumber, 0, "own team number");
    INIT_ENUM_RW(teamColor, TeamColor::RED, "own team color");
    INIT_VAR_RW(score, 0, "own team's score");
    INIT_VAR_RW(penaltyShot, 0, "penalty shot counter");
    INIT_VAR_RW(singleShots, 0, "bits represent penalty shot success");
    INIT_VAR_RW(messageBudget, 1200, "number of team messages the team is allowed to send for the remainder of the game");

    INIT_VAR(penalties, penaltiesArray(), "array of all teammates penalty states");
    INIT_VAR(opponentPenalties, penaltiesArray(),
             "array of all opponents penalty states");

    INIT_VAR_RW(opponentTeamNumber, 0, "opponent's team number");
    INIT_ENUM_RW(opponentTeamColor, TeamColor::BLUE, "opponent's team color");
    INIT_VAR_RW(opponentScore, 0, "opponent's score");
    INIT_VAR_RW(opponentPenaltyShot, 0, "opponent's penalty shot counter");
    INIT_VAR_RW(opponentSingleShots, 0,
                "bits represent opponent's penalty shot success");
    INIT_VAR_RW(opponentMessageBudget, 0, "number of team messages the team is allowed to send for the remainder of the game");

    INIT_VAR(lastPacketTs, -1, "timestamp of last valid gamecontroller packet");
}

std::ostream &operator<<(std::ostream &s, const GamecontrolBlackboard *rhs) {
    s << "current gamecontrol blackboard content:\n";
    s << "  competitionType:     " << int(rhs->competitionType)     << "\n";
    s << "  competitionPhase:    " << int(rhs->competitionPhase)    << "\n";
    s << "  gameState:           " << int(rhs->gameState)           << "\n";
    s << "  gamePhase:           " << int(rhs->gamePhase)           << "\n";
    s << "  firstHalf:           " << rhs->firstHalf                << "\n";
    s << "  firstHalf:           " << rhs->firstHalf                << "\n";
    s << "  firstHalf:           " << rhs->firstHalf                << "\n";
    s << "  firstHalf:           " << rhs->firstHalf                << "\n";
    s << "  firstHalf:           " << rhs->firstHalf                << "\n";
    s << "  firstHalf:           " << rhs->firstHalf                << "\n";
    s << "  kickoff:             " << rhs->kickoff                  << "\n";
    s << "  secsRemaining:       " << rhs->secsRemaining            << "\n";
    s << "  secondaryTime:       " << rhs->secondaryTime            << "\n";
    s << "  penalty:             " << int(rhs->penalty)             << "\n";
    s << "  secsTillUnpenalised: " << rhs->secsTillUnpenalised      << "\n";
    s << "  teamNumber:          " << rhs->teamNumber               << "\n";
    s << "  teamColor:           " << int(rhs->teamColor)           << "\n";
    s << "  score:               " << rhs->score                    << "\n";
    s << "  penaltyShot:         " << rhs->penaltyShot              << "\n";
    s << "  singleShots:         " << rhs->singleShots              << "\n";
    s << "  opponentTeamNumber:  " << rhs->opponentTeamNumber       << "\n";
    s << "  opponentTeamColor:   " << int(rhs->opponentTeamColor)   << "\n";
    s << "  opponentScore:       " << rhs->opponentScore            << "\n";
    s << "  opponentPenaltyShot: " << rhs->opponentPenaltyShot      << "\n";
    s << "  opponentSingleShots: " << rhs->opponentSingleShots      << "\n";
    return s;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
