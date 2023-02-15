#include "tcpvideosource.h"

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>
#include <cstddef>
#include <thread>
#include <chrono>

#include <framework/util/assert.h>
#include <framework/logger//logger.h>
#include <framework/util/getenv.h>
#include <framework/network/network.h>

#include <representations/bembelbots/constants.h>
#include <representations/camera/camera.h>

static const std::string_view MAGIC{"wbimage"};
static constexpr size_t IMG_SIZE{camera::w * camera::h * camera::bpp};

using namespace std;
using namespace std::chrono;
using NetworkPorts = Network::NetworkPorts;

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::ip::address_v4;

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

struct header_t {
    char magic[8];
    uint16_t tick;
    uint8_t camId;
    uint8_t bpp;
    uint16_t width;
    uint16_t height;
};
static constexpr size_t BUF_SIZE{IMG_SIZE + sizeof(header_t)};

class TCPWrapper {
private:
    boost::asio::io_service io;
    tcp::socket sock;
    tcp::endpoint ep;

public:
    explicit TCPWrapper(const boost::asio::ip::tcp::endpoint &e) : sock(io), ep(e) {}

    void connect() {
        boost::system::error_code ec;
        int c{0};

        sock.close();
        do {
            sock.connect(ep, ec);
            if (ec) {
                sock.close();
                sleep_for(10ms);
            }
        } while (ec && (++c < 50));
    }

    void disconnect() { sock.close(); }

    void send(const char *buf, const size_t &size, boost::system::error_code &ec) {
        sock.send(boost::asio::buffer(buf, size), 0, ec);
        if (ec)
            sock.close();
    }

    size_t recv(char *buf, const size_t &size, boost::system::error_code &ec) {
        auto r = sock.receive(boost::asio::buffer(buf, size), 0, ec);
        if (ec)
            sock.close();
        return r;
    }

    bool is_open() { return sock.is_open(); }
};

TCPVideoSource::TCPVideoSource(const int &cam_id, std::string simulatorHost, bool docker) {
    isTopCam = (cam_id == TOP_CAMERA);
    uint16_t port{0};
    address ip;

    if (docker) {
        const std::string lola_port{getEnvVar("LOLA_PORT")},
              sim_host{getEnvVar("SIMULATOR_HOST")};
        jsassert(!lola_port.empty()) << "set LOLA_PORT environment variable for docker mode!";
        jsassert(!sim_host.empty()) << "set SIMULATOR_HOST environment variable for docker mode!";
        ip = address::from_string(sim_host);
        port = std::stoi(lola_port) + 1 + cam_id; // tcam = LOLA_PORT+1, bcam = LOLA_PORT+2
    } else {
        port = (isTopCam) ? NetworkPorts::TCP_TCAM : NetworkPorts::TCP_BCAM;
        ip = simulatorHost.empty() ? address_v4::loopback() : address_v4::from_string(simulatorHost);
    }
    tcp = new TCPWrapper(tcp::endpoint(ip, port));
}

TCPVideoSource::~TCPVideoSource() {
}

void TCPVideoSource::fetchImage(CamImage &dst) {
    boost::system::error_code ec;
    size_t read_bytes{0};
    header_t h;

    if (!tcp->is_open())
        tcp->connect();

    // read header
    read_bytes = tcp->recv(reinterpret_cast<char *>(&h), sizeof(h), ec);
    if (ec)
        return;

    assert(read_bytes == sizeof(header_t)); // incomplete header

    // sanity checks
    assert(h.magic == MAGIC);
    assert(h.camId == getCamera());
    assert(h.bpp == camera::bpp);
    assert(h.width == camera::w);
    assert(h.height == camera::h);

    // read image
    read_bytes = 0;
    while (read_bytes < IMG_SIZE) {
        read_bytes += tcp->recv(reinterpret_cast<char *>(dst.data + read_bytes), IMG_SIZE - read_bytes, ec);
        if (ec)
            return;
    }
    assert(read_bytes == IMG_SIZE); // incomplete data

    dst.camera = getCamera();
    dst.timestamp = h.tick * CONST::lola_cycle_ms;
}

bool TCPVideoSource::startCapturing() {
    return true;
}

int TCPVideoSource::getCamera() const {
    return !isTopCam;
}

void TCPVideoSource::setCamera(const int camera) {
    WARN_NOT_IMPLEMENTED;
}

int TCPVideoSource::getParameter(CameraOption option) {
    return 0;
}

void TCPVideoSource::setParameter(CameraOption option, int value) {
}

bool TCPVideoSource::setCameraResolution(const int /*res*/) {
    WARN_NOT_IMPLEMENTED;
    return true;
}

int TCPVideoSource::getCameraResolution() const {
    WARN_NOT_IMPLEMENTED;
    return 0;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
