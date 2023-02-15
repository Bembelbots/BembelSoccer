option(body_control) {

    common_transition {
        if (!fall_control) {
            if (fallen) {
                goto do_nothing;    
            } else {
                goto restiff;
            }
        } else if (!standing_up) {
            switch (fallen_side) {
            case FallenSide::FRONT: goto standup_from_front;
            case FallenSide::BACK: goto standup_from_back;
            case FallenSide::NONE: goto do_nothing; 
            }
        } else {
            goto do_nothing;
        }
    } 

    initial_state(do_nothing) {}

    state(prepare_standup) {
        transition {
            switch (fallen_side) {
            case FallenSide::FRONT: goto standup_from_front;
            case FallenSide::BACK: goto standup_from_back;
            case FallenSide::NONE: goto do_nothing; 
            }
        }
    }

    state(restiff) {
        action {
            stiffnessCmd = StiffnessCommand::RESTIFF_ALL;
        }
    }

    state(standup_from_back) {
        transition {
            if (!fallen && motion_active == Motion::NONE) {
                goto stand;
            }
        }
        action {
            stiffnessCmd = StiffnessCommand::RESTIFF_ALL;
            do_motion(Motion::STAND_UP_FROM_BACK);
        }
    }

    state(standup_from_front) {
        action {
            stiffnessCmd = StiffnessCommand::RESTIFF_ALL;
            do_motion(Motion::STAND_UP_FROM_FRONT);
        }
    }

    state(stand) {
        action { do_motion(Motion::STAND); }
    }

}
