//
// Created by felix on 21.04.18.
//
option(do_motion, (Motion)motion, (bool)(false)_high_stand) {

    initial_state(set_motion) {
        action {
            bm_type = motion;
            high_stand = _high_stand;
        }
    }
}
