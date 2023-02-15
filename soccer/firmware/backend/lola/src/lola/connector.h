#pragma once

#include <array>
#include <atomic>

#include <msgpack.hpp>
#include <boost/asio.hpp>

#include <libbembelbots/monitor_shm.hpp>
#include <libbembelbots/bembelbots_shm.h>

namespace lola {

class Connector {

public:
    using socket_t = boost::asio::generic::stream_protocol::socket;
    using endpoint_t = boost::asio::generic::stream_protocol::endpoint;
    Connector(Connector::socket_t &s, const Connector::endpoint_t &e);

    void connect();

    // worker, loops until stop() is called
    void run();

    // signal worker to stop
    void stop();

    static msgpack::sbuffer stiffnessRequest(const float &stiffness);

private:
    static constexpr size_t LOLA_PKT_SIZE{896};

    socket_t &sock;
    endpoint_t ep;

    std::atomic<bool> running{false};
    bool simulator{false}, frontendConnected{false}, shutdown{false}, sitDone{false}, calibrated{false};
    int64_t timestamp;

    BembelbotsShm bb_shm;
    MonitorShm m_shm;

    std::array<float, lbbNumOfSensorIds> sensors{0};
    std::array<float, lbbNumOfActuatorIds> actuators{0};

    RobotName robotName{RobotName::UNKNOWN};
    struct {
        std::string head{"<unknown>"};
        std::string body{"<unknown>"};
    } serial;

    void calibrateGyro();
    void setIdleLeds();

    void toggleFrontend(const bool start);
    void buttonHandler();

    void readSensors(const msgpack::object &obj);
    msgpack::sbuffer &writeActuators();

    static void say(const std::string &text);

    void stiffness(const float &v);
};

} // namespace lola

// vim: set ts=4 sw=4 sts=4 expandtab:
