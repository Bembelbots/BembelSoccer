#pragma once

#include <bodycontrol/internals/submodule.h>

#include <framework/math/angle.h>

class Odometry : public SubModule {
public:
    SubModuleReturnValue step(BodyBlackboard *bb) override {

        // ensure this module is currently not running...
        assert(0);

        // add angle to bodyAngles computed by IMU
        auto odo = bb->odometry;
        odo.angle = Rad{bb->bodyAngles(2)};

        if (bb->activeMotion != Motion::WALK) {
            return RUNNING;
        }

        auto walkPosition = &bb->walkPosition;
        auto walkMaxSpeed = &bb->walkMaxSpeed;

        // equal calculation for the 'covered distance'
        float trans_x = (walkPosition->coord.y / walkMaxSpeed->at(0)) * x_per_10;
        float trans_y = -(walkPosition->coord.x / walkMaxSpeed->at(1)) * y_per_10;

        // walk towards the turned direction 'walk_alpha' (by adding alpha)
        float walk_alpha = odo.angle.posNormalized();

        // do 2-d translation of coord sys
        float add_x = cosf(walk_alpha) * trans_x - sinf(walk_alpha) * trans_y;
        float add_y = sinf(walk_alpha) * trans_x + cosf(walk_alpha) * trans_y;

        odo.coord.add({add_x, add_y});

        return RUNNING;
    }

private:
    const float x_per_10 = 0.03376;
    const float y_per_10 = 0.012;
};
