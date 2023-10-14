#pragma once

#include <array>
#include <mutex>
#include <stdint.h>

#include <framework/rt/module.h>
#include <framework/common/platform.h>
#include <framework/network/udp.h>
#include <representations/motion/body_state.h>
#include <representations/spl/SPLStandardMessage.h>
#include <representations/spl/RoboCupGameControlData.h>
#include <representations/blackboards/worldmodel.h>
#include <representations/blackboards/gamecontrol.h>
#include <representations/blackboards/settings.h>
#include <representations/teamcomm/commands.h>
#include <representations/teamcomm/teammessage.h>
#include <representations/teamcomm/types.h>
#include <representations/debugserver/debugstate.h>
#include <representations/whistle/whistleresult.h>

#include <team_message_generated.h>

class Robot;
class Ball;
class WorldModel;

class TeamComm : public rt::Module {
public:
    static constexpr int numPlayers{MAX_NUM_PLAYERS};

    // interval tuning
    static constexpr TimestampMs defaultBcastIntervalMs{20000}; ///< default interval
    static constexpr TimestampMs fastInterval{2500};            ///< something happend of immediate importance
    static constexpr TimestampMs mediumInterval{5000};          ///< not critical, but teammates should know about it
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
    rt::Input<Snapshot<GamecontrolBlackboard>> gc_message;
    rt::Input<Snapshot<WorldModelBlackboard>> world;
    rt::Input<BodyState> body;
    rt::Output<TeamMessage, rt::Event> team_message;
    rt::Command<TeamcommCommand, rt::Handle> cmds;
    rt::Input<DebugState> debugState;
    rt::Input<WhistleResult, rt::Snoop> whistle;

    bbapi::TeamMessageT tm;
    std::atomic<int> msgCount{0}; // number of all sent & received messages, in case GC is not working
    std::array<TimestampMs, numPlayers> msgTimestamp; // timestamp of last message received

    bool isActive(const int &id);
    
    // adjust message interval based on worldmodel changes
    float calcInterval(const Robot &r, const Ball &b);

    void handle(TeamcommDebugInfo &);

    void broadcast(const Robot &r, const Ball &b);
    void netRecv(const char *data, const size_t &bytes_recvd,
                 const udp::endpoint &sender);

};


// vim: set ts=4 sw=4 sts=4 expandtab:
