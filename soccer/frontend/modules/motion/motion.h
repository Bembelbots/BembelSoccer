#pragma once
#include <framework/rt/module.h>
#include <representations/motion/body_commands.h>
#include <representations/blackboards/settings.h>
#include <representations/motion/body_state.h>
#include <representations/bembelbots/feature_flags.h>
#include "../nao/naostate.h"
#include "bodycontrol/blackboards/body_interface.h"

class BodyControl;

class MotionModule : public rt::NoThreadModule {
public:
    void connect(rt::Linker &) override;
    void load(rt::Kernel &) override;
    void setup() override;
private:
    std::shared_ptr<rt::NoThreadModule> nao;
    std::shared_ptr<BodyControl> bc;

    rt::Context<SettingsBlackboard> settings;
    rt::Output<BodyState, rt::Event> body_state;
    rt::Command<BodyCommand, rt::Handle> cmds;

    BodyInterface interface;

    void tick(NaoState &state);
};
