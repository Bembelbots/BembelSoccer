option(walk_machine, (WalkAction) walkAction) {
    /*
       walk thetha: <0 => turn left
       >0 => turn right
       */

    common_transition {
        switch (walkAction) {
        case FORWARDS:
            goto goforwards;
        case BACKWARDS:
            goto gobackwards;
            break;
        case TURN:
            goto goturn;
            break;
        case TURNLEFT:
            goto goturnleft;
            break;
        case TURNRIGHT:
            goto goturnright;
            break;
        case TIPPLE:
            goto gotipple;
            break;
        case ALIGN:
            goto goalign;
            break;
        case SIDE_RIGHT:
            goto gosideright;
            break;
        case SIDE_LEFT:
            goto gosideleft;
            break;
        case TURN_AROUND:
            goto goturnaround;
            break;
        case OMNIDIRECTIONAL:
            goto regular;
            break;

        case INSTEP_KICK_LEFT:
            goto instep_kick_left;

        case INSTEP_KICK_RIGHT:
            goto instep_kick_right;

        case RAW:
            goto goraw;

        }
    }

    initial_state(start) {}

    // regular walk (omni-directional) forward as fast as possible
    state(regular) {
        action {
            walk_x = 1.0f;
            walk_y = 0.0f;
            // here realize "closed control-loop 'damped-wave' walk"
            walk_theta = scale(bm_target_pos.angle().deg(), -80.0f, 80.0f, -1.0f, 1.0f);
            walk_action = OMNIDIRECTIONAL;
            do_motion(Motion::WALK);
        }
    }

    state(goforwards) {
        action {
            walk_x = 0.75f;
            walk_y = 0.0f;
            walk_theta = 0.0f;
            arms = ArmsPosition::BACK;
            walk_action = FORWARDS;
            do_motion(Motion::WALK);
        }
    }

    state(goturnaround) {
        action {
            walk_x = 0.0f;
            walk_y = 0.65f;

            // always keep dist at 0.4f
            //walk_theta = bm_target_pos.dist();
            walk_theta = -scale(bm_target_pos.dist(), -1.0f, 1.0f, -1.0f, 1.0f);
            //LOG_INFO << "walk theta:"<<walk_theta;
            //LOG_INFO<<"target pos:" <<bm_target_pos;
            walk_action = TURN;
            do_motion(Motion::WALK);
        }
    }

    state(gosideright) {
        action {
            walk_x = 0.0f;
            walk_y = -0.95f;
            walk_theta = 0.f;
            walk_action = SIDE_RIGHT;
            do_motion(Motion::WALK);
        }
    }

    state(gosideleft) {
        action {
            walk_x = 0.0f;
            walk_y = 0.95f;
            walk_theta = 0.f;
            walk_action = SIDE_LEFT;
            do_motion(Motion::WALK);
        }
    }

    // turn in-place, reducing turn speed the nearer the target gets
    // bm_target_angle negativ -> turn left, target_angle positiv ->turn right
    state(goturn) {
        action {
            walk_x = 0.0f;
            walk_y = 0.0f;
            walk_theta = scale(bm_target_angle.deg(), -90.0f, 90.0f, -1.0f, 1.0f);
            //LOG_INFO<<"walk_theta :"<<walk_theta;
            arms = ArmsPosition::BACK;
            walk_action = TURN;
            do_motion(Motion::WALK);
        }
    }

    // turn in-place, reducing turn speed the nearer the target gets
    // bm_target_angle negativ -> turn left, target_angle positiv ->turn right
    state(goturnleft) {
        action {
            walk_x = 0.0f;
            walk_y = 0.0f;
            walk_theta = 1.0f;
            //LOG_INFO<<"walk_theta :"<<walk_theta;
            arms = ArmsPosition::BACK;
            walk_action = TURN;
            do_motion(Motion::WALK);
        }
    }

    state(goturnright) {
        action {
            walk_x = 0.0f;
            walk_y = 0.0f;
            walk_theta = -1.0f;
            //LOG_INFO<<"walk_theta :"<<walk_theta;
            arms = ArmsPosition::BACK;
            walk_action = TURN;
            do_motion(Motion::WALK);
        }
    }


    // align to target (without turning), scaling x/y walk speed
    state(goalign) {
        action {
            walk_x = scale(bm_target_pos.x, -0.5f, 0.5f, -1.0f, 1.0f);
            walk_y = scale(bm_target_pos.y, -0.3f, 0.3f, -1.0f, 1.0f);

            walk_theta = 0.0f;
            arms = ArmsPosition::BACK;
            walk_action = ALIGN;
            do_motion(Motion::WALK);
        }
    }

    // walk backwards
    state(gobackwards) {
        action {
            walk_x = -0.75f;
            walk_y = 0.0f;
            walk_theta = 0.0f;
            arms = ArmsPosition::BACK;
            walk_action = BACKWARDS;
            do_motion(Motion::WALK);
        }
    }

    // fast stop and stay in tipple
    state(gotipple) {
        action {
            walk_x = 0.0f;
            walk_y = 0.0f;
            walk_theta = 0.0f;
            walk_action = TIPPLE;
            do_motion(Motion::WALK);
        }
    }

    state(goraw) {
        action {
            walk_action = RAW;
            do_motion(Motion::WALK);
        }
    }

    state(instep_kick_left) {
        action {
            walk_action = INSTEP_KICK_LEFT;
            do_motion(Motion::WALK);
        }
    }

    state(instep_kick_right) {
        action {
            walk_action = INSTEP_KICK_RIGHT;
            do_motion(Motion::WALK);
        }
    }

}
