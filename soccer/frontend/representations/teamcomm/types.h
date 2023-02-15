#pragma once

// intentions as defined in SPL message header
enum class Intention {
    NONE,
    GOALKEEPER,
    DEFENDER,
    PLAY_BALL,
    LOST
};

// suggestions as defined in SPL message header
enum class Suggestion {
    NONE,
    GOALKEEPER,
    DEFENSE,
    OFFENSE,
    PLAY_BALL
};
