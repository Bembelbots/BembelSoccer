#pragma once

#include <array>
#include <mutex>
#include <stdint.h>

#include <framework/rt/module.h>
#include <framework/common/platform.h>
#include <framework/network/udp.h>
#include <representations/motion/body_state.h>
#include <representations/blackboards/worldmodel.h>
#include <representations/blackboards/settings.h>
#include <representations/teamcomm/commands.h>
#include <representations/teamcomm/teammessage.h>
#include <representations/teamcomm/types.h>
#include <representations/debugserver/debugstate.h>

#include <gamecontrol_generated.h>
#include <team_message_generated.h>
#include <whistle_message_generated.h>
#include <referee_gesture_message_generated.h>

class Robot;
class Ball;
class WorldModel;

class TeamComm : public rt::Module {
public:
    static constexpr int numPlayers{MAX_NUM_PLAYERS};

    // interval tuning
    static constexpr TimestampMs defaultBcastIntervalMs{15000}; ///< default interval
    static constexpr TimestampMs fastInterval{1500};            ///< something happend of immediate importance
    static constexpr TimestampMs mediumInterval{2500};          ///< not critical, but teammates should know about it
    static constexpr TimestampMs immediateInterval{0};          ///< not critical, but teammates should know about it
    
    static constexpr float       thresholdMovement{0.3f};       ///< bot moved at least this distance (unit: m)
    static constexpr float       thresholdBallPosition{0.1f};   ///< don't prioritize ball when it has not moved (unit: m)
    static constexpr float       thresholdBallDist{2.f};        ///< don't prioritize ball when it is farther away (unit: m)
    static constexpr TimestampMs thresholdBallAge{1000};        ///< consider ball recent, when own ballAge is below this value (unit: ms)
    static constexpr TimestampMs thresholdTeamBallAge{16000};   ///< always send if now robot has seen ball (unit: ms)

    void connect(rt::Linker &) override;
    void process() override;
    void setup() override;

private:
    TeamcommDebugInfo debug;

    std::shared_ptr<UDP> net;
    rt::Context<SettingsBlackboard> settings;
    rt::Command<TeamcommCommand, rt::Handle> cmds;
    rt::Input<bbapi::GamecontrolMessageT, rt::Listen> gamecontrol;
    rt::Input<Snapshot<WorldModelBlackboard>> world;
    rt::Input<BodyState> body;
    rt::Input<DebugState> debugState;
    rt::Input<bbapi::WhistleMessageT, rt::Snoop> whistle;
    rt::Input<bbapi::RefereeGestureMessageT, rt::Snoop> refereeGesture;
    rt::Output<TeamMessage, rt::Event> team_message;
    rt::Output<bbapi::TeamMessageT, rt::Event> team_message_log;

    bbapi::TeamMessageT tm;
    std::atomic<int> msgCount{0}; // number of all sent & received messages, in case GC is not working
    std::array<TimestampMs, numPlayers> msgTimestamp; // timestamp of last message received

    bool isActive(const int &id);
    
    Robot teamMsg2robot(const bbapi::TeamMessageT &msg);
    Ball  teamMsg2ball(const int &sender, const bbapi::pos &pos, const uint8_t &conf, const int &age = 0);

    // adjust message interval based on worldmodel changes
    float calcInterval(const Robot &r, const Ball &b);

    void handle(TeamcommDebugInfo &);

    void broadcast(const Robot &r, const Ball &b);
    void netRecv(const char *data, const size_t &bytes_recvd,
                 const udp::endpoint &sender);

};


// vim: set ts=4 sw=4 sts=4 expandtab:
