#pragma once

#include <framework/rt/module.h>

#include <representations/blackboards/settings.h>
#include <representations/teamcomm/teammessage.h>

#include <gamestate_message_generated.h>
#include <gamecontrol_generated.h>
#include <referee_gesture_message_generated.h>
#include <whistle_message_generated.h>

class Gamestate : public rt::Module {
public:
    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;

private:
    static constexpr int SACRIFICE_PLAYER_NUM{4};
    static constexpr int SACRIFICE_DELAY_SEC{15};

    rt::Context<SettingsBlackboard> settings;
    rt::Input<bbapi::GamecontrolMessageT> game_control;
    rt::Input<bbapi::WhistleMessageT, rt::Snoop> whistle;
    rt::Input<bbapi::RefereeGestureMessageT, rt::Snoop> referee_gesture;
    rt::Input<TeamMessage, rt::Snoop> team_message;
    rt::Output<bbapi::GamestateMessageT> output;

    void handleOfficialStandby();
    TimestampMs gestureDetectionTimestamp;
    bool gestureInStandbyDetected;
    bool gestureInStandbyPenalized;
};
