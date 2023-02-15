option(led_gamestate) {

    common_transition {
        if (is_unstiff) {
            goto unstiffState;
        } else if (is_penalized) {
            goto penalizedState;
        } else {
            switch (game_state_led) {
            case GameState::INITIAL:
                goto initialState;
                break;
            case GameState::READY:
                goto readyState;
                break;
            case GameState::SET:
                goto setState;
                break;
            case GameState::PLAYING:
                goto playingState;
                break;
            case GameState::FINISHED:
                goto finishedState;
                break;
            }
        }
    }

    initial_state(initialState) {
        action {
            chest_color = RGBColor::OFF;
        }
    }

    state(readyState) {
        action {
            chest_color = RGBColor::BLUE;
        }
    }

    state(setState) {
        action {
            chest_color = RGBColor::YELLOW;
        }
    }

    state(playingState) {
        action {
            chest_color = RGBColor::GREEN;
        }
    }

    state(finishedState) {
        action {
            chest_color = RGBColor::OFF;
        }
    }

    state(penalizedState) {
        action {
            chest_color = RGBColor::RED;
        }
    }

    state(unstiffState) {
        action {
            // crappy blinking animation
            chest_color = (state_time/1000 & 1) ? RGBColor::BLUE : RGBColor::OFF;
        }
    }
}
