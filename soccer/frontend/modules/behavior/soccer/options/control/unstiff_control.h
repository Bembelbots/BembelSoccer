option(unstiff_control) {
    initial_state(idle) {
        transition {
            if (is_unstiff) {
                goto sit;
            }
        }
        action {
            is_sitting = false;
        }
    }

    state(sit) {
        transition {
            if (state_time > 1500)
                goto unstiff;
        }
        action {
            is_sitting = true;
            bm_type = Motion::SIT;
            stiffnessCmd = StiffnessCommand::FORCE_STIFF;
        }
    }

    state(unstiff) {
        transition {
            if (!is_unstiff)
                goto restiff;
        }
        action { stiffnessCmd = StiffnessCommand::FORCE_UNSTIFF; }
    }

    state(restiff) {
        transition {
            if (state_time > 1000)
                goto standup;
        }
        action { stiffnessCmd = StiffnessCommand::FORCE_STIFF; }
    }

    state(standup) {
        transition {
            if (is_standing)
                goto idle;
        }
        action {
            bm_type = Motion::INTERPOLATE_TO_STAND;
        }
    }
}
