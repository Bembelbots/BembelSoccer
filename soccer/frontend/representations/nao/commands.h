#pragma once
#include <framework/rt/message_utils.h>

struct NaoCommand {};

struct SetPitchOffset {
    int camera;
    float offset;
};

RT_REGISTER_COMMAND(SetPitchOffset, NaoCommand);
