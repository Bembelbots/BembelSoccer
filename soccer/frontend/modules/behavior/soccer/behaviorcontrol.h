#pragma once

#include <framework/rt/module.h>
#include <representations/blackboards/worldmodel.h>
#include <representations/blackboards/gamecontrol.h>
#include <representations/motion/body_state.h>
#include <representations/motion/body_commands.h>
#include <representations/whistle/whistleresult.h>
#include <representations/teamcomm/commands.h>

#include "behavior.h"
#include "framework/rt/endpoints.h"

class SettingsBlackboard;

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
    rt::Input<Snapshot<GamecontrolBlackboard>> gamecontrolBb;
    rt::Input<BodyState> body;
    rt::Input<WhistleResult, rt::Snoop> whistle;

    //rt::Output<DynamicRoleMessage> dynamicRole;
    rt::Command<BodyCommand> bodyCmds;
    rt::Command<WhistleCommand> whistleCmds;
    rt::Command<TeamcommCommand> teamcommCmds;

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
