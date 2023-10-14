option(wait_for_motion, (Motion)motion, (int)(10000)max_time) {

    common_transition {
        if (option_time > max_time) {
            goto motion_took_too_long;
        }
    }
    
    initial_state(set_motion) {
        transition {
            if (motion_active == motion) {
                goto wait_until_motion_finished;
            } else if (state_time > 0) {
                goto wait_until_my_motion_starts;
            }
        }
        action {
            do_motion(motion);
        }
    }

    state(wait_until_my_motion_starts) {
        transition {
            if (motion_active == motion) {
                goto wait_until_motion_finished;
            }
        }
    }

    state(wait_until_motion_finished) {
        transition {
            if (motion_active != motion) {
                goto finished;
            }
        }
    }

    target_state(finished) {}

    aborted_state(motion_took_too_long) {}
}
