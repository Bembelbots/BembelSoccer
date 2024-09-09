#include "gamestate.h"

#include <string>
#include <queue>

#include <framework/util/frame.h>
#include <framework/thread/util.h>
#include <framework/logger/logger.h>

using bbapi::GameState;

void Gamestate::connect(rt::Linker &link) {
    link.name = "Gamestate";
    link(settings);
    link(game_control);
    link(whistle);
    link(referee_gesture);
    link(team_message);
    link(output);
}

void Gamestate::setup() {
    output->gameStateOfficial = bbapi::GameState::INITIAL;
    output->gameStateReal = bbapi::GameState::INITIAL;
}

void Gamestate::process() {
    FrameScope frame(30ms);

    output->gameStateOfficial = game_control->gameState;

    switch (game_control->gameState) {
        // We can trust these states.
        case GameState::INITIAL:
        case GameState::READY:
        case GameState::FINISHED:
            output->gameStateReal = game_control->gameState;
            break;

        case GameState::STANDBY:
            handleOfficialStandby();
            break;

        /* TODO: We can't trust these states.
         * However currently the behavior decides this by itself so
         * for now lets pretend that it's the real state. Do not
         * trust it!
         */
        case GameState::SET:
        case GameState::PLAYING:
            output->gameStateReal = game_control->gameState;
            break;
    }
}

void Gamestate::handleOfficialStandby() {
    /* This state machine could be rebuilt using CABSL. */

    if (output->gameStateReal != GameState::STANDBY && output->gameStateReal != GameState::READY) {
        // Clear any detections that might have happened earlier
        referee_gesture.fetch();
        team_message.fetch();
        gestureDetectionTimestamp = -10 * 1000;
        gestureInStandbyDetected = false;
        gestureInStandbyPenalized = false;
        output->gameStateReal = GameState::STANDBY;
    }

    if (!gestureInStandbyDetected) {
        for (const auto &gesture : referee_gesture.fetch()) {
            if (gesture.leftArmUp && gesture.rightArmUp) {
                gestureDetectionTimestamp = getTimestampMs();
                gestureInStandbyDetected = true;
                LOG_DEBUG << "[Gamestate]: Saw referee.";
                break;
            }
        }
        for (const auto &message : team_message.fetch()) {
            if (message.refereeGestureUp) {
                gestureDetectionTimestamp = getTimestampMs();
                gestureInStandbyDetected = true;
                LOG_DEBUG << "[Gamestate]: Team mate saw referee.";
                break;
            }
        }
    }

    if (gestureInStandbyDetected && !gestureInStandbyPenalized && output->gameStateReal != GameState::READY) {
        static const int delayUntilWalkIn = (settings->id == SACRIFICE_PLAYER_NUM - 1) ? -1 : SACRIFICE_DELAY_SEC * 1000;
        const TimestampMs now = getTimestampMs();
        if ((now - gestureDetectionTimestamp) > delayUntilWalkIn) {
            if (game_control->myTeamInfo) {
                for (size_t i = 0; i < game_control->myTeamInfo->players.size(); ++i) {
                    const auto &player = game_control->myTeamInfo->players.at(i);
                    if (player && player->penalty == bbapi::Penalty::ILLEGAL_MOTION_IN_STANDBY) {
                        gestureInStandbyPenalized = true;
                        break;
                    }
                }
            }

            if (!gestureInStandbyPenalized) {
                LOG_DEBUG << "[Gamestate]: Setting real game state to READY.";
                output->gameStateReal = GameState::READY;
            } else {
                LOG_DEBUG << "[Gamestate]: A robot was penalized during ready gesture delay, not setting READY state.";
            }
        }
    }
}
