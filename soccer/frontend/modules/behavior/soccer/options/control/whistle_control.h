// control whistle detection:
//   * activate whistel detection in SET/PLAYING states
//   * deactivate in all other states
//   * reset whistle flag after 20s

option(whistle_constrol) {
    initial_state(initial) {
        transition {
            switch (game_state) {
                case GameState::SET:
                case GameState::PLAYING:
                    goto listen;
                default:
                    goto initial;
            }
        }
        action {
            whistle_listen = false;
            whistle = false;
        }
    }

    state(listen) {
        transition {
            switch (game_state) {
                case GameState::SET:
                case GameState::PLAYING:
                    break;
                default:
                    goto initial;
            }
            if (whistle)
                goto whistle;
        }
        action {
            whistle_listen = true;
        }
    }

    state(whistle) {
        transition {
            if (state_time > 20e3)
                goto initial;
        }
    }
}
