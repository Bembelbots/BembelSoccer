#pragma once
#include "whistle_commands_generated.h"
#include "whistle_message_generated.h"
#include <framework/rt/module.h>
#include <representations/blackboards/settings.h>
#include <representations/motion/body_commands.h>

class SimpleBehavior : public rt::Module {
public:
    void setup() override;
    void connect(rt::Linker &) override;
    void process() override;
    void stop() override;
private:
    rt::Context<SettingsBlackboard> settings;
    rt::Command<bbapi::WhistleCommandT> whistle;
    rt::Command<BodyCommand> motion;
    rt::Input<bbapi::WhistleMessageT, rt::Snoop> whistle_result;
    bool once = false;
};
