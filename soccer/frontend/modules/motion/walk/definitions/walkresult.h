#pragma once
#include "../htwk/kinematics/PositionPlain.h"

struct WalkResult {
    float waddle_left;
    float waddle_right;
    float step_height;
    kinematics::PositionPlain feet;
};
