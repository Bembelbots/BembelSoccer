#pragma once
#include <framework/rt/module.h>
#include <representations/blackboards/settings.h>
#include <representations/whistle/commands.h>
#include <representations/whistle/whistleresult.h>
#include <representations/motion/body_commands.h>

class SimpleBehavior : public rt::Module {
public:
    void setup() override;
    void connect(rt::Linker &) override;
    void process() override;
    void stop() override;
private:
    rt::Context<SettingsBlackboard> settings;
    rt::Dispatch<WhistleCommand> whistle;
    rt::Dispatch<BodyCommand> motion;
    rt::Input<WhistleResult, rt::Snoop> whistle_result;
    bool once = false;
};
