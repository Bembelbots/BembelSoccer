option(root) {
    common_transition {
        if (is_unstiff || is_sitting) {
            goto unstiff;
        }
    }

    initial_state(begin) {
        transition {
            if (is_standing) {
                goto play;
            }
        }
        action { do_motion(Motion::INTERPOLATE_TO_STAND); }
    }

    state(play) {
        whistle_constrol();

        game();

        //hm_type = HeadMotionType::NONE;
        body_control();
        head_control();
        arm_control();
        led_control();
    }

    state(unstiff) {
        transition {
            if (!is_unstiff && !is_sitting)
                goto play;
        }

        action {
            led_control();
            unstiff_control();
        }
    }
}
