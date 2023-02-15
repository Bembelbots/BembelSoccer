#pragma once

#include <cstring>
#include <cstdarg>
#include <vector>

#include <bodycontrol/internals/submodule.h>

#include <bodycontrol/utils/state_machine.h>
#include <representations/motion/body_commands.h>
#include <representations/bembelbots/types.h>

#include "../../../walk/walking_engine.h"

// Rewriting the BembelWalk is better - but we lack time.
class WalkWrapper : public SubModule {
public:
    void setup(Setup s) override {
        s.cmds->connect<WalkMotion, &WalkWrapper::handleRequest>(this);
        s.cmds->connect<StopWalking, &WalkWrapper::handleRequest>(this);
        s.cmds->connect<HighStand, &WalkWrapper::handleRequest>(this);
        s.cmds->connect<NormalStand, &WalkWrapper::handleRequest>(this);
        s.cmds->connect<Tipple, &WalkWrapper::handleRequest>(this);

        walk.reset();
    }

    SubModuleReturnValue step(BodyBlackboard *bb) override {

        if (wasReset) {
            //walk.setBodyHeight(BodyHeight::HIGH);
            //walk.setStand(true);
            wasReset = false;
        }

        walk.proceed(bb);

        bb->qns[IS_WALKING] = !walk.isStanding();
        bb->qns[IS_STANDING] = walk.isStanding();

        return MOTION_STABLE;
    }

    void handleRequest(const WalkCommand &cmd) {
        walk.setDirection(cmd.speed);
        walk.setStiffness(cmd.stiffness);
        walk.setStand(cmd.stand);
        walk.setBodyHeight(cmd.height);
        activateMe = true;
    }

    void reset() override {
        wasReset = true;
        walk.reset();
    }

private:
    WalkingEngine walk;
    bool wasReset = false;
};
