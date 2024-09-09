#pragma once

#include <cmath>

#include <bodycontrol/internals/submodule.h>
#include <framework/logger/logger.h>

#include <libbembelbots/sit.hpp>

class Sit : public SubModule {

public:
    SubModuleReturnValue step(BodyBlackboard *bb) {
        bb->qns[IS_STANDING] = false;

        if (sitDone)
            return MOTION_STABLE;

        sitDone = sit(bb->sensors, *bb->actuators);

        return MOTION_UNSTABLE;
    }

    void reset() {
        sitDone = false;
    }

private:
    bool sitDone{false};
};

// vim: set ts=4 sw=4 sts=4 expandtab:
