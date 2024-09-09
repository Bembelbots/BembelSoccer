#pragma once

#include <framework/rt/module.h>
#include <representations/blackboards/worldmodel.h>
#include <representations/motion/body_state.h>
#include <representations/motion/body_commands.h>
#include <representations/teamcomm/commands.h>

#include "behavior.h"
#include "framework/rt/flags.h"
#include "framework/rt/endpoints.h"

#include "gamecontrol_generated.h"
#include "whistle_commands_generated.h"
#include "whistle_message_generated.h"
#include "gamestate_message_generated.h"

class SettingsBlackboard;
class ReactiveWalkBlackboard;

namespace BEHAVE_PRIVATE {

class BehaviorControl : public rt::Module {

public:
    ~BehaviorControl();

    void connect(rt::Linker &) override;

    void setup() override;

    void process() override;

    void run(microTime time_ms);

private:
    rt::Context<SettingsBlackboard> settings;
    rt::Context<PlayingField> playingField;
    rt::Input<Snapshot<WorldModelBlackboard>, rt::Require> worldBb;
    rt::Input<bbapi::GamecontrolMessageT, rt::Listen> gamecontrol;
    rt::Input<BodyState> body;
    rt::Input<bbapi::WhistleMessageT, rt::Snoop> whistle;
    rt::Input<bbapi::GamestateMessageT> gameState;

    //rt::Output<DynamicRoleMessage> dynamicRole;
    rt::Command<BodyCommand> bodyCmds;
    rt::Command<bbapi::WhistleCommandT> whistleCmds;
    rt::Command<TeamcommCommand> teamcommCmds;

    std::shared_ptr<ReactiveWalkBlackboard> reactivewalkboard;
    std::shared_ptr<Behavior> _myBehavior;

    std::vector<Behavior::OptionInfos::Option> roots;

    ////
    // Update Behavior here so it doesn't depend on the blackboards.
    // This is required for the standalone behavior.
    ///

    // update Behavior from other blackboards
    void updateBehavior();

    // apply LEDs from behavior
    void applyLEDs();

    // apply motion stuff from behavior
    void applyMotion();

    // apply misc stuff from behavior
    void applyMisc();
};

} // namespace BEHAVE_PRIVATE

// vim: set ts=4 sw=4 sts=4 expandtab:
