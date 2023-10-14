option(led_control) {
    initial_state(idle) {
        action {
            led_gamestate();
        }
    }

}

