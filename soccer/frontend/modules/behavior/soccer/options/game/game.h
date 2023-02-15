option(game) {
    common_transition {
        if (is_penalized) {
            goto penalizedState;
        } else {
            switch (game_state) {
            case GameState::READY:
                goto readyState;
                break;
            case GameState::SET:
                if (whistle) {
                    goto playingState;
                } else {
                    goto setState;
                }
                break;
            case GameState::PLAYING:
                if (whistle && (bot_pos.coord.x > playingfield->_lengthInsideBounds/4.f) && (ball_age > C::max_ball_age)) {
                    goto readyState;
                } else {
                    goto playingState;
                }
                break;
            case GameState::FINISHED:
                goto finishedState;
                break;
            case GameState::INITIAL:
            default:
                goto initialState;
                break;
            }
        }
    }

    initial_state(initialState) {
        action {
            fall_control = false;

            do_motion(Motion::STAND, true);
            hm_type = HeadMotionType::NONE;
            hm_pos = Coord(1, 0); // faces forward

            arms = ArmsPosition::SIDE;
        }
    }

    state(readyState) {
        action {
            game_state_led = GameState::READY;
            fall_control = true;
            
            //game_ready();
        }
    }

    state(setState) {
        action {
            fall_control = true;
            was_penalized = false;
           
            do_motion(Motion::STAND);
            hm_type = HeadMotionType::SWEEP;
            arms = ArmsPosition::SIDE;
        }
    }

    state(playingState) {
        action {
            game_state_led = GameState::PLAYING;
            fall_control = true;
            
            //loca_head_motion();
            //game_play();
            
        }
    }

    state(finishedState) {
        action {
            fall_control = false;

            do_motion(Motion::STAND, true);
            hm_type = HeadMotionType::NONE;
        }
    }

    state(penalizedState) {
        action  {
            fall_control = false;
            was_penalized = true;
            
            do_motion(Motion::STAND, true);
            hm_type = HeadMotionType::NONE;
        }
    }
}
