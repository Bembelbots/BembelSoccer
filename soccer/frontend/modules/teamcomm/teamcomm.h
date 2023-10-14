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

class Robot;
class Ball;
class WorldModel;

class TeamComm : public rt::Module {
public:
    static constexpr int numPlayers{MAX_NUM_PLAYERS};

    // interval tuning
    static constexpr TimestampMs defaultBcastIntervalMs{10000}; ///< default interval
    static constexpr TimestampMs fastInterval{1000};            ///< something happend of immediate importance
    static constexpr TimestampMs mediumInterval{2000};          ///< not critical, but teammates should know about it
    static constexpr TimestampMs immediateInterval{0};          ///< not critical, but teammates should know about it
    
    static constexpr float       thresholdMovement{0.3f};       ///< bot moved at least this distance (unit: m)
    static constexpr float       thresholdBallPosition{0.1f};   ///< don't prioritize ball when it has not moved (unit: m)
    static constexpr float       thresholdBallDist{2.f};        ///< don't prioritize ball when it is farther away (unit: m)
    static constexpr TimestampMs thresholdBallAge{1000};        ///< consider ball recent, when own ballAge is below this value (unit: ms)
    static constexpr TimestampMs thresholdTeamBallAge{16000};   ///< always send if now robot has seen ball (unit: ms)

    TeamComm();
    virtual ~TeamComm();
    
    void connect(rt::Linker &) override;
    void process() override;
    void setup() override;

    /**
     * send SPL message broadcasts (rate limited according to SPL rules)
     */
    void broadcast(const Robot &r, const Ball &b);

    /**
     * update the worldmodel from received messages.
     * This ensures the worldmodel data is only changed once
     * per cognition tick, otherwise the behavior may end up
     * in undefined state, if data changes during decision making
     */
    void updateWorldModel();

    /**
     * callback for receiving data
     */
    void netRecv(const char *data, const size_t &bytes_recvd,
                 const udp::endpoint &sender);

    /**
     * check if robot with a specific ID is active
     */
    bool isActive(const int &id);


private:
    std::shared_ptr<UDP> net;
    //GamecontrolBlackboard *gc;
    rt::Context<SettingsBlackboard> settings;
    rt::Input<Snapshot<GamecontrolBlackboard>> gc_message;
    rt::Input<Snapshot<WorldModelBlackboard>> world;
    rt::Input<BodyState> body;
    rt::Output<TeamMessage, rt::Event> team_message;
    rt::Dispatch<TeamcommCommand, rt::Handle> cmds;
    rt::Input<DebugState> debugState;
    rt::Input<WhistleResult, rt::Snoop> whistle;

    std::atomic<int> msgCount{0}; // number of all sent & received messages, in case GC is not working
    std::recursive_mutex mtx; // mutex for locking data access

    std::array<uint32_t, numPlayers> seq; // sequence numbers of packets
    std::array<uint32_t, numPlayers> ack; // acks received by other bots

    std::array<SPLStandardMessage, numPlayers> msg; // last message received
    std::array<TimestampMs, numPlayers> msgTimestamp; // timestamp of last message received

    Robot spl2robot(const SPLStandardMessage
                    &spl); // extract robot data from SPL messages
    
    // adjust message interval based on worldmodel changes
    float calcInterval(const Robot &r, const Ball &b);
};


// vim: set ts=4 sw=4 sts=4 expandtab:
