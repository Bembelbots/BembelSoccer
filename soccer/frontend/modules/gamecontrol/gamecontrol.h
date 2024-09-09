#pragma once

#include "framework/rt/endpoints/context.h"
#include "framework/rt/flags.h"
#include "representations/blackboards/gamecontrol.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <framework/rt/module.h>
#include <framework/common/platform.h>
#include <framework/network/udp.h>
#include <framework/blackboard/snapshot.h>
#include <representations/spl/RoboCupGameControlData.h>
#include <representations/blackboards/settings.h>
#include <representations/motion/body_state.h>
#include <representations/blackboards/worldmodel.h> 

#include <gamecontrol_generated.h>

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

private:
    rt::Context<GamecontrolBlackboard, rt::Write> bb;
    rt::Context<SettingsBlackboard, rt::Write> settings;
    rt::Input<BodyState, rt::Snoop> body_state;
    rt::Input<Snapshot<WorldModelBlackboard>> world;
    rt::Output<bbapi::GamecontrolMessageT, rt::Event> gc_event;

    std::shared_ptr<UDP> net;
    std::mutex mtx;

    bbapi::GamecontrolMessageT gc_data;
    RoboCupGameControlReturnData gc_return;
    std::atomic<int> teamIndex;

    void buttonHandler(const BodyState &);
    
    void updateBB();
    void parsePacket(const RoboCupGameControlData &pkt);
};

// vim: set ts=4 sw=4 sts=4 expandtab:
