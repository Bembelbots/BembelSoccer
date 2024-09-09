#pragma once

#include <array>
#include <atomic>

#include <flatbuffers/array.h>
#include <msgpack.hpp>
#include <boost/asio.hpp>

#include <libbembelbots/monitor_shm.hpp>
#include <libbembelbots/bembelbots_shm.h>
#include <msgpack/v2/object_fwd.hpp>
#include <msgpack/v2/object_fwd_decl.hpp>

#include <ipc_sensor_message_generated.h>
#include <ipc_actuator_message_generated.h>

namespace lola {

class Connector {

public:
    using socket_t = boost::asio::generic::stream_protocol::socket;
    using endpoint_t = boost::asio::generic::stream_protocol::endpoint;
    Connector(Connector::socket_t &s, const Connector::endpoint_t &e);

    void connect();

    // worker, loops until stop() is called
    // @return: true if system shutdown was requested
    bool run();

    // signal worker to stop
    void stop();

    static msgpack::sbuffer stiffnessRequest(const float &stiffness);

private:
    static constexpr size_t LOLA_PKT_SIZE{896};

    socket_t &sock;
    endpoint_t ep;

    std::atomic<bool> running{false};
    bool shutdown{false}, sitDone{false}, calibrated{false};
    uint32_t last_tick{std::numeric_limits<uint32_t>::max()}, connCnt{0};


    BembelbotsShm bb_shm;
    MonitorShm m_shm;

    bbapi::BembelIpcSensorMessageT sensorMsg;
    bbapi::BembelIpcActuatorMessageT actuatorMsg;
    bbipc::Sensors &sensors;
    bbipc::Actuators &actuators;

    void calibrateGyro();
    void setIdleLeds();
    void setEarLeds();

    void toggleFrontend(const bool start) const;
    void buttonHandler();

    void readSensors(const msgpack::object &obj);
    msgpack::sbuffer &writeActuators();

    static void say(const std::string &text);
};

} // namespace lola

// vim: set ts=4 sw=4 sts=4 expandtab:
