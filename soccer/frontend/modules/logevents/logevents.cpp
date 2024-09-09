#include "logevents.h"
#include "log_event_generated.h"

using bbapi::FallenState;

void LogEvents::setup() {
}

void LogEvents::connect(rt::Linker &link) {
    link.name = "LogEvents";

    link(body);
    link(gamecontrol);
    link(event);
}

void LogEvents::process() {
    if (gc_state.gameState != gamecontrol->gameState || gc_state.penalized != gamecontrol->penalized ||
            gc_state.unstiff != gamecontrol->unstiff) {
        gc_state = gamecontrol;
        bbapi::LogEventT ev;
        bbapi::GameControlEventT gev;

        gev.gameState = gamecontrol->gameState;
        gev.penalized = gamecontrol->penalized;
        gev.unstiff = gamecontrol->unstiff;

        ev.event.Set(gev);
        event.emit(ev);
        
        std::string s;
        if (gev.penalized)
            s += " (penalized)";
        if (gev.unstiff)
            s += " (unstiff)";
        LOG_DEBUG << "LogEvent: GameState changed " << EnumNameGameState(gev.gameState) << s;

        return;
    }

    for (const auto &i : {IS_STANDING_UP, IS_FALLEN, IS_FALLING}) {
        if (body->qns[i] && body_qns[i] != body->qns[i]) {
            body_qns = body->qns;
            bbapi::LogEventT ev;
            bbapi::FallenEventT fev;
            switch (i) {
                case IS_STANDING_UP:
                    fev.state = FallenState::GettingUp;
                    break;
                case IS_FALLING:
                    fev.state = FallenState::Falling;
                    break;
                case IS_FALLEN:
                    fev.state = FallenState::Fallen;
                    break;
                default:
                    jsassert(false);
            }
            fev.groundContact = body->qns[HAS_GROUND_CONTACT];

            ev.event.Set(fev);
            event.emit(ev);
            
            LOG_DEBUG << "LogEvent: BodyState changed " << EnumNameFallenState(fev.state);

            return;
        }
    }
}
