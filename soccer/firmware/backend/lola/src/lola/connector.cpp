#include <cmath>
#include <cstdio>
#include <chrono>
#include <limits>
#include <memory>
#include <thread>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <string_view>

#include <unistd.h>
#include <sys/wait.h>

#include <Eigen/Core>

#include "connector.h"

#include <framework/ipc/time.h>
#include <framework/logger/logger.h>

#include <libbembelbots/sit.hpp>
#include <libbembelbots/tts.hpp>
#include <libbembelbots/button.hpp>
#include <libbembelbots/config/botnames.h>
#include <libbembelbots/config/calibration.h>

#include <representations/flatbuffers/types/actuators.h>
#include <representations/flatbuffers/types/sensors.h>

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

namespace lola {

static constexpr std::array<std::string_view, 11> LOLA_ACTUATOR_NAMES{
        "Position", "Stiffness", "REar", "LEar", "Chest", "LEye", "REye", "LFoot", "RFoot", "Skull", "Sonar"};

static inline std::string to_string(const msgpack::object &o) {
    return std::string(o.via.str.ptr, o.via.str.size);
}

bool forkexec(const char *cmd, const char *arg = nullptr) {
    pid_t pid = fork();
    if (pid == 0) { // child
        // no spam
        freopen("/dev/null", "w", stdout); // cppcheck-suppress ignoredReturnValue
        freopen("/dev/null", "w", stderr); // cppcheck-suppress ignoredReturnValue
        // exec
        execl(cmd, cmd, arg, nullptr);
        exit(0);
    }
    return (pid > 0);
}

template<uint16_t length>
void mutate_sensors(::flatbuffers::Array<float, length> *arr, const std::vector<float> &values) {
    assert(arr->size() == values.size());
    int i{0};
    for (const auto &v : values)
        arr->Mutate(i++, v);
}

msgpack::sbuffer Connector::stiffnessRequest(const float &stiffness) {
    msgpack::sbuffer buffer;
    buffer.clear();
    msgpack::packer<msgpack::sbuffer> pk(&buffer);

    pk.pack_map(2);

    // set chest button leds to yellow
    pk.pack("Chest");
    pk.pack_array(3);
    pk.pack(1.f);
    pk.pack(1.f);
    pk.pack(0.f);

    // set all stiffnesses to -1
    pk.pack("Stiffness");
    pk.pack_array(LOLA_NUMBER_OF_JOINTS);

    for (size_t i = 0; i < LOLA_NUMBER_OF_JOINTS; ++i)
        pk.pack(-1.f);

    return buffer;
}

template<size_t N>
void msgpackPack(msgpack::packer<msgpack::sbuffer> &pk, const std::array<float, N> &a) {
    pk.pack_array(a.size());
    for (const auto &i : a)
        pk.pack_float(i);
}

template<size_t N>
void msgpackPack(msgpack::packer<msgpack::sbuffer> &pk, const bbipc::LEDString<N> &eye) {
    pk.pack_array(3 * N);
    for (const auto &i : eye.r)
        pk.pack_float(i);
    for (const auto &i : eye.g)
        pk.pack_float(i);
    for (const auto &i : eye.b)
        pk.pack_float(i);
}

void msgpackPack(msgpack::packer<msgpack::sbuffer> &pk, const bbipc::LEDSingle &led) {
    pk.pack_array(3);
    pk.pack_float(led.r);
    pk.pack_float(led.g);
    pk.pack_float(led.b);
}

Connector::Connector(Connector::socket_t &s, const Connector::endpoint_t &e)
  : sock(s)
  , ep(e)
  , bb_shm(bembelbotsShmName, true)
  , m_shm(true)
  , sensors(sensorMsg.sensors)
  , actuators(actuatorMsg.actuators) {

    sensorMsg.tick = 0;
    sensorMsg.timestamp = 0;
    sensorMsg.lolaTimestamp = 0;
    sensorMsg.robotConfig = std::make_unique<bbapi::RobotConfigT>();
    sensorMsg.robotConfig->head = std::make_unique<bbapi::HardwareIdT>();
    sensorMsg.robotConfig->body = std::make_unique<bbapi::HardwareIdT>();
}

void Connector::connect() {
    boost::system::error_code ec;

    std::cout << "Connecting to LoLA socket";
    do {
        std::cout << "." << std::flush;
        sock.connect(ep, ec);
        if (ec) {
            // connection error
            sock.close();
            sleep_for(500ms);
        }
    } while (running && ec);
    std::cout << std::endl;

    // this avoids abruptly changing joint positions,
    // when frontend is not running
    sitDone = false;
}

bool Connector::run() {
    static constexpr size_t length{LOLA_PKT_SIZE * 5};
    char pkt[length];
    boost::system::error_code ec;

    running = true;

    connect();

    // received SIGINT/SIGTERM while waiting for socket
    if (!running)
        return false;

    bool robotConfigReceived{false};

    while (running) {
        size_t pkt_len = sock.receive(boost::asio::buffer(pkt, length), 0, ec);
        if (ec) {
            std::cerr << __PRETTY_FUNCTION__ << " - socket error, reconnecting: " << ec.message() << std::endl;
            connect();
            continue;
        }

        // update timestamp immediately after packet is received
        sensorMsg.lolaTimestamp = getSystemTimestamp();
        if (pkt_len < LOLA_PKT_SIZE) {
            std::cerr << __PRETTY_FUNCTION__ << " - received incomplete LoLa msgpack object" << std::endl;
            continue;
        }

        // create msgpack object & parse sensor values
        msgpack::object_handle oh = msgpack::unpack(pkt, pkt_len);
        msgpack::object obj = oh.get();

        readSensors(obj);

        if (!robotConfigReceived) {
            const auto &rc{*sensorMsg.robotConfig};
            std::cout << std::endl;
            std::cout << "LoLa connection established on robot \"" << DEFS::enum2botname(sensorMsg.robotName) << "\""
                      << std::endl;
            std::cout << "Head serial: " << rc.head->serial << std::endl;
            std::cout << "Body serial: " << rc.body->serial << std::endl;
            std::cout << std::endl;
            robotConfigReceived = true;
        }

        // generate msgpack object & write to socket
        const auto &buf{writeActuators()};
        sock.send(boost::asio::buffer(buf.data(), buf.size()), 0, ec);
        if (ec) {
            std::cerr << __PRETTY_FUNCTION__ << " - socket error, reconnecting: " << ec.message() << std::endl;
            connect();
            continue;
        }

        // update monitor SHM values
        m_shm.updateSensors(sensorMsg);

        // check chest button
        buttonHandler();

        static int c{0};
        if (shutdown && sitDone) {
            // delay shutdown so LED requests will still be sent
            if (++c < 2)
                continue;
            // frontend has stopped & bot is sitting, save to power off
            say("Poweroff.");
            running = false;
        }

        // power off when on battery & charge is less than 10%
        if (sensors.battery.current < 0) {
            if (sensors.battery.charge < 0.25f) {
                using namespace std::chrono;
                using namespace std::chrono_literals;
                using clock = std::chrono::steady_clock;

                static time_point<clock> lastWarn;
                auto timeNow{clock::now()};

                if ((timeNow - lastWarn) > 60s) {
                    say("Low battery, please charge me!!!");
                    lastWarn = timeNow;
                }
            }

            if (sensors.battery.charge < 0.1f) {
                say("Low battery!");
                toggleFrontend(false);
                shutdown = true;
            }
        }

        // kill zombies
        waitpid((pid_t)-1, 0, WNOHANG);
    }

    return (shutdown && !sensorMsg.simulator);
}

void Connector::stop() {
    running = false;
}

void Connector::calibrateGyro() {
    using namespace Eigen;

    static constexpr int CALIB_COUNT{200};
    static Vector3f prevGyro, gyroOffset, gyroGain;
    static int count;

    if (sensorMsg.simulator)
        return; // don't apply any calibration in webots simulator

    auto &bodySerial = sensorMsg.robotConfig->body->serial;
    if (gyroGain == Vector3f(0, 0, 0)) {
        if (calibration::body.count(bodySerial)) {
            gyroGain = calibration::body.at(bodySerial).gyroGain;
            std::cerr << "Using gyro gain calibration: [" << gyroGain[0] << ", " << gyroGain[1] << ", " << gyroGain[2]
                      << "]" << std::endl;
        } else {
            gyroGain = {1, 1, 1};
            std::cerr << "ERROR: No gyro calibration found for " << bodySerial << std::endl;
        }
    }

    Vector3f g = sensors.imu.gyroscope;
    if (!calibrated) {
        Vector3f d = prevGyro - g;
        // WARNING: LoLa weirdness: we only get new gyro values with every 2nd frame
        if (d != Vector3f(0, 0, 0)) {
            prevGyro = g;

            if ((d.dot(d)) < 0.0002f) {
                gyroOffset += g / CALIB_COUNT;
                if (count == CALIB_COUNT) {
                    std::cout << "Gyro offset calibrated." << std::endl;
                    calibrated = true;
                    count = 0;
                }
                ++count;
            } else {
                count = 0;
                gyroOffset = {0, 0, 0};
            }
        }
    } else {
        g -= gyroOffset;
    }

    // these axis are flipped compared to V5 robots
    sensors.imu.accelerometer.y() *= -1;
    sensors.imu.gyroscope.z() *= -1;

    sensors.imu.gyroscope.array() *= gyroGain.array();
}

void Connector::toggleFrontend(const bool start) const {
    if (start) {
        if (forkexec("/home/nao/bin/jsfrontend")) {
            say("Starting frontend.");
        } else {
            std::cerr << __PRETTY_FUNCTION__ << " - fork() failed, cannot start frontend" << std::endl;
            say("Failed to start frontend.");
        }
    } else {
        if (sensorMsg.connected)
            say("Stopping frontend.");
        forkexec("/usr/bin/killall", "jsfrontend");
    }
}

void Connector::buttonHandler() {
    switch (checkBtnEvent(sensors.touch.chest.button)) {
        case BtnClk::SINGLE:
            if (!sensorMsg.connected)
                toggleFrontend(true);
            break;
        case BtnClk::TRIPPLE:
            toggleFrontend(!sensorMsg.connected);
            break;
        case BtnClk::LONG:
            if (!shutdown) {
                say("System shutdown.");
                toggleFrontend(false);
                shutdown = true;
            }
            break;
        case BtnClk::CAM_RESET:
            say("Camera reset.");
            system("/opt/aldebaran/libexec/reset-cameras.sh toggle");
            system("pkill -9 jsfrontend");
            system("pkill -9 lola");
            exit(42);
        default:
            break;
    }
}

void Connector::readSensors(const msgpack::object &obj) {
    for (const auto &e : obj.via.map) {
        // enumerate
        const auto &key{to_string(e.key)};
        const auto &val{e.val};

        if (key == "Stiffness") {
            sensors.joints.stiffness = val.as<bbipc::JointArray>();
        } else if (key == "Position") {
            sensors.joints.position = val.as<bbipc::JointArray>();
        } else if (key == "Temperature") {
            sensors.joints.temperature = val.as<bbipc::JointArray>();
        } else if (key == "Current") {
            sensors.joints.current = val.as<bbipc::JointArray>();
        } else if (key == "Status") {
            sensors.joints.status = val.as<bbipc::JointStatus>();
        } else if (key == "Battery") {
            const auto &[charge, status, current, temp] = val.as<std::array<float, 4>>();
            sensors.battery = {.charge = charge, .current = current, .status = status, .temperature = temp};
        } else if (key == "Accelerometer") {
            const auto &[x, y, z] = val.as<std::array<float, 3>>();
            sensors.imu.accelerometer << x, y, z;
        } else if (key == "Gyroscope") {
            const auto &[x, y, z] = val.as<std::array<float, 3>>();
            sensors.imu.gyroscope << x, y, z;
        } else if (key == "Angles") {
            const auto &[x, y] = val.as<std::array<float, 2>>();
            sensors.imu.angles << x, y;
        } else if (key == "Sonar") {
            const auto &[l, r] = val.as<std::array<float, 2>>();
            sensors.sonar = {l, r};
        } else if (key == "FSR") {
            const auto &[lfl, lfr, lrl, lrr, rfl, rfr, rrl, rrr] = val.as<std::array<float, 8>>();
            sensors.fsr.left.arr = {lfl, lfr, lrl, lrr};
            sensors.fsr.right.arr = {rfl, rfr, rrl, rrr};
        } else if (key == "Touch") {
            const auto &a = val.as<std::array<float, 14>>();
            sensors.touch.chest.button = a[0];
            sensors.touch.head = {a[1], a[2], a[3]};
            sensors.touch.feet.left = {a[4], a[5]};
            sensors.touch.hands.left = {a[6], a[7], a[8]};
            sensors.touch.feet.right = {a[9], a[10]};
            sensors.touch.hands.right = {a[11], a[12], a[13]};
        } else if (key == "RobotConfig") {
            const auto &[bodyId, bodyVersion, headId, headVersion] = val.as<std::array<std::string, 4>>();
            auto robotName = DEFS::headid2name(headId);

            sensorMsg.robotName = DEFS::headid2name(headId);
            sensorMsg.simulator = (robotName == RobotName::SIMULATOR);
            sensorMsg.backendVersion = BB_BACKEND_VERSION;

            auto &rc = *sensorMsg.robotConfig;
            rc.head->serial = headId;
            rc.head->version = headVersion;
            rc.body->serial = bodyId;
            rc.body->version = bodyVersion;
        } else {
            std::cerr << "Error: unhandled key '" << key << "' in LoLa sensor message!" << std::endl;
            exit(69);
        }
    }

    calibrateGyro();

    auto &ipc{bb_shm->sensors.consumedData()};
    if (!ipc.set(sensorMsg)) {
        std::cerr << "Error: sensor message flatbuffer exceeds SHM size!" << std::endl;
        exit(4);
    }

    // swap SHM buffers
    bb_shm->sensors.consume();

    // increase sensor tick
    ++sensorMsg.tick;

    //std::cerr << std::endl << obj << std::endl;
}

void Connector::setIdleLeds() {
    static const std::vector<float> gamma{0, 0.01, 0.05, 0.15, 0.3, 0.45, 0.6, 0.8, 1, 1};
    static size_t c{0};
    static int idx{0}, inc{1};

    actuators.led.skull.fill(gamma[idx]);

    // blink brain leds as soon as gyro is calibrated
    if (c > gamma.size() && calibrated) {
        c = 0;
        idx += inc;
        if (idx < 0) {
            idx = 0;
            inc = 1;
        } else if (idx >= static_cast<int>(gamma.size())) {
            idx = gamma.size() - 1;
            inc = -1;
        }
    }
    ++c;

    // chest button: green when charging, off on battery
    if (sensors.battery.current > 0)
        actuators.led.chest = {0, 0.25f, 0};
    else
        actuators.led.chest = {0, 0, 0};

    // eyes: white when running, off when shutdown has been initiated
    actuators.led.eyes.left.fill(!shutdown);
    actuators.led.eyes.right.fill(!shutdown);
}

void Connector::setEarLeds() {
    // ears: used as battery indicator
    const size_t n{actuators.led.ears.left.size()};
    for (size_t i = 0; i < n; ++i) {
        actuators.led.ears.left[i] = actuators.led.ears.right[i] =
                (sensors.battery.charge > (i - 1) / float(n)) * !shutdown;
    }
}

msgpack::sbuffer &Connector::writeActuators() {
    bb_shm->actuators.timedProduce(11);
    const auto &ipc{bb_shm->actuators.producedData()};

    // use temporary buffer, to avoid issues with old data in tripple buffer if frontend is not running
    bbapi::BembelIpcActuatorMessageT am;
    if (!ipc.get(am)) {
        // SHM does not contain valid flatbuffer after creation, so silently ignore error until we are connected
        std::cerr << "Error: invalid actuator message flatbuffer received." << std::endl;
        exit(2);
    }

    // check if frontend has increased actuator tick
    if (am.tick != last_tick) {
        last_tick = actuatorMsg.tick;
        actuatorMsg = am;
        connCnt = std::min(connCnt + 1, 15U);
    } else if (connCnt > 0) {
        --connCnt;
    }

    // assume frontend is connected after 10 ticks
    if (!sensorMsg.connected && (connCnt > 10)) {
        sensorMsg.connected = true;
        std::cout << "New frontend connection established." << std::endl;
    } else if (sensorMsg.connected && (connCnt == 0)) {
        sensorMsg.connected = false;
        std::cout << "Lost frontend connection." << std::endl;
    }

    if (sensorMsg.connected) {
        sitDone = false;
    } else { // no frontend connected
        // set status LEDs
        setIdleLeds();

        // check if torso is upright
        if (std::max(std::abs(sensors.imu.angles.x()), std::abs(sensors.imu.angles.y())) < 0.3f) {
            // move to sit pose
            if (!sitDone)
                sitDone = sit(sensors, actuators);
        } else {
            // unstiff all joints
            actuators.joints.stiffness.fill(-1);
            actuators.joints.position = sensors.joints.position;
            sitDone = false;
        }
    }
    
    setEarLeds();

    // create msgpack buffer
    static msgpack::sbuffer buffer;
    buffer.clear();
    msgpack::packer<msgpack::sbuffer> pk(&buffer);

    pk.pack_map(LOLA_ACTUATOR_NAMES.size());

    for (const auto &name : LOLA_ACTUATOR_NAMES) {
        pk.pack(name);
        if (name == "Position") {
            msgpackPack(pk, actuators.joints.position);
        } else if (name == "Stiffness") {
            msgpackPack(pk, actuators.joints.stiffness);
        } else if (name == "REar") {
            msgpackPack(pk, actuators.led.ears.right);
        } else if (name == "LEar") {
            msgpackPack(pk, actuators.led.ears.left);
        } else if (name == "Chest") {
            msgpackPack(pk, actuators.led.chest);
        } else if (name == "LEye") {
            msgpackPack(pk, actuators.led.eyes.left);
        } else if (name == "REye") {
            msgpackPack(pk, actuators.led.eyes.right);
        } else if (name == "LFoot") {
            msgpackPack(pk, actuators.led.feet.left);
        } else if (name == "RFoot") {
            msgpackPack(pk, actuators.led.feet.right);
        } else if (name == "Skull") {
            msgpackPack(pk, actuators.led.skull);
        } else if (name == "Sonar") {
            pk.pack_array(2);
            pk.pack(actuators.sonar.left);
            pk.pack(actuators.sonar.right);
        } else {
            std::cerr << "Error: no handler for actuator chain '" << name << "' defined!" << std::endl;
            exit(3);
        }
    }

    //std::cerr << std::endl << msgpack::unpack(buffer.data(), buffer.size()).get() << std::endl;
    return buffer;
}

void Connector::say(const std::string &text) {
    static TTSSpeechd tts("lola-backend");

    std::cout << "[TTS] " << text << std::endl;
    tts.say(text, SPD_IMPORTANT);
}

} // namespace lola

// vim: set ts=4 sw=4 sts=4 expandtab:
