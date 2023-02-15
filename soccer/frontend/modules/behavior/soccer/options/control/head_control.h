option(head_control) {

    initial_state(waitidle) {
        transition {
            if (hm_type == HeadMotionType::BALL) {
                if (ball_age < 1000) {
                    hm_pos = ball_rcs_pos;
                } else {
                    hm_pos = Coord(1, 0);
                }
            }
        }
    }
}