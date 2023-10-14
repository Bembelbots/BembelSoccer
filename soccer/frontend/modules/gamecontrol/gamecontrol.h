#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <framework/rt/module.h>
#include <framework/common/platform.h>
#include <framework/network/udp.h>
#include <framework/blackboard/snapshot.h>
#include <representations/spl/RoboCupGameControlData.h>
#include <representations/blackboards/settings.h>
#include <representations/blackboards/gamecontrol.h>
#include <representations/motion/body_state.h>
#include <representations/blackboards/worldmodel.h> 

// forward declarations
class GamecontrolMessage;
class SensorBlackboard;

class Gamecontrol : public rt::Module {
public:
    Gamecontrol();

    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;

    void recv(const char *msg, const size_t &bytes_recvd,
              const boost::asio::ip::udp::endpoint &sender);

    void updateBB();

private:
    rt::Context<GamecontrolBlackboard, rt::Write> bb;
    rt::Context<SettingsBlackboard, rt::Write> settings;
    rt::Output<Snapshot<GamecontrolBlackboard>> output;
    rt::Input<BodyState, rt::Snoop> body_state;
    rt::Input<Snapshot<WorldModelBlackboard>> world;

    std::shared_ptr<UDP> net;
    std::mutex mtx;

    RoboCupGameControlData gc_data;
    RoboCupGameControlReturnData gc_return;
    int teamIndex;
    std::atomic<TimestampMs> lastReceived;

    std::atomic<bool> isUnstiff;
    std::atomic<bool> isPenalized;

    void penalize();
    void unpenalize();
    void checkPenalty(bool manual);
    void buttonHandler(const BodyState &);
};

// vim: set ts=4 sw=4 sts=4 expandtab:
