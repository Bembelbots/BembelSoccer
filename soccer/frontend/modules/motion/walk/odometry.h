#pragma once

#include <cmath>

#include "htwk/utils/point_3d.h"
#include "htwk/utils/stl_ext.h"

#define DISABLE_ANGLE_OPERATORS
#include <framework/math/directed_coord.h>

class Odometry {
public:
    float scale_x = 1.0f;
    float scale_y = 1.0f;

    void apply(float dx, float dy, float gyroYaw) { 
        float gyroDiff = normalizeRotation(gyroYaw - lastGyroYaw);
        lastGyroYaw = gyroYaw;
        pos = pos.walk(DirectedCoord(dx * scale_x, dy * scale_y, Rad{gyroDiff}));
    }

    DirectedCoord getMovementVec() {
        return pos;
    }

private:
    float lastGyroYaw = 0;
    DirectedCoord pos;
};
