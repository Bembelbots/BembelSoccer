#pragma once

#include <framework/util/enum/serializable_enum.h>

// clang-format off
SERIALIZABLE_ENUM_CLASS(Motion, 
        (NONE), 
        (KICK_LEFT), 
        (KICK_RIGHT), 
        (GOALIE_THROW_LEFT), 
        (GOALIE_THROW_RIGHT), 
        (STAND),
        (INTERPOLATE_TO_STAND),
        (STAND_UP_FROM_BACK), 
        (STAND_UP_FROM_FRONT), 
        (WALK), 
        (DESIGNER),
        (SIT),
        (RAISE_ARM)
);
// clang-format on
