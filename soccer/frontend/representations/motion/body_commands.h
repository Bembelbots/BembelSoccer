#pragma once

#include <framework/rt/message_utils.h>
#include <framework/math/coord.h>
#include <framework/math/directed_coord.h>
#include "motion.h"
#include "../bembelbots/types.h"

struct BodyCommand {};

struct SetHeadMotion {
    HeadMotionType value;
};
RT_REGISTER_COMMAND(SetHeadMotion, BodyCommand);

struct SetHeadLookRCS {
    Coord value;
};
RT_REGISTER_COMMAND(SetHeadLookRCS, BodyCommand);

struct DoMotion {
    Motion id;
};
RT_REGISTER_COMMAND(DoMotion, BodyCommand);

struct SetStiffness {
    StiffnessCommand cmd;
};
RT_REGISTER_COMMAND(SetStiffness, BodyCommand);

struct SetLeds {
    LED group;
    int color;
};
RT_REGISTER_COMMAND(SetLeds, BodyCommand);

struct WalkCommand {
    DirectedCoord speed;
    float stiffness;
    bool stand;
    BodyHeight height;
};

struct WalkMotion : public WalkCommand {
    WalkMotion(DirectedCoord speed) {
        this->speed = speed;
        stiffness = 0.8f;
        stand = false;
        height = BodyHeight::NORMAL;
    }
};
RT_REGISTER_COMMAND(WalkMotion, BodyCommand);

struct StopWalking : public WalkCommand {
    StopWalking() {
        speed = {};
        stiffness = 0.8f;
        stand = true;
        height = BodyHeight::NORMAL;
    }
};
RT_REGISTER_COMMAND(StopWalking, BodyCommand);

struct HighStand : public WalkCommand {
    HighStand() {
        speed = {};
        stiffness = 0.3f;
        stand = true;
        height = BodyHeight::HIGH;
    }
};
RT_REGISTER_COMMAND(HighStand, BodyCommand);

struct NormalStand : public WalkCommand {
    NormalStand() {
        speed = {};
        stiffness = 0.3f;
        stand = true;
        height = BodyHeight::NORMAL;
    }
};
RT_REGISTER_COMMAND(NormalStand, BodyCommand);

struct Tipple : public WalkCommand {
    Tipple() {
        speed = {};
        stiffness = 0.8f;
        stand = false;
        height = BodyHeight::NORMAL;
    }
};
RT_REGISTER_COMMAND(Tipple, BodyCommand);
