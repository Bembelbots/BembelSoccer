#include <cmath>
#include <cstdio>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <string_view>

#include <unistd.h>
#include <sys/wait.h>

#include <Eigen/Core>

#include <libbembelbots/sit.hpp>
#include <libbembelbots/tts.hpp>
#include <libbembelbots/button.hpp>
#include <libbembelbots/config/calibration.h>
#include <libbembelbots/config/botnames.h>

#include "connector.h"

#include "mapping.h"

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

namespace lola {

static inline std::string to_string(const msgpack::object &o) {
    return std::string(o.via.str.ptr, o.via.str.size);
}

bool forkexec(const char *cmd, const char *arg = nullptr) {
    pid_t pid = fork();
    if (pid == 0) { // child
        // no spam
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
        // exec
        execl(cmd, cmd, arg, nullptr);
        exit(0);
    }
    return (pid > 0);
}

msgpack::sbuffer Connector::stiffnessRequest(const float &stiffness) {
    static const size_t NUM_JOINTS{lola::mapping::actuators.at("Stiffness").size()};
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
    pk.pack_array(NUM_JOINTS);

    for (size_t i = 0; i < NUM_JOINTS; ++i)
        pk.pack(-1.f);

    return buffer;
}

Connector::Connector(Connector::socket_t &s, const Connector::endpoint_t &e)
  : sock(s), ep(e), timestamp(-1), bb_shm(bembelbotsShmName + "_9559", true), m_shm(true) {
    static_assert(lbbNumOfSensorIds == sizeof(sensorNames) / sizeof(*sensorNames));
    static_assert(lbbNumOfActuatorIds == sizeof(actuatorNames) / sizeof(*actuatorNames));
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

void Connector::run() {
    static constexpr size_t length{LOLA_PKT_SIZE * 5};
    char pkt[length];
    boost::system::error_code ec;

    running = true;

    connect();

    // received SIGINT/SIGTERM while waiting for socket
    if (!running)
        return;

    size_t pkt_len = sock.receive(boost::asio::buffer(pkt, length));
    msgpack::object_handle oh = msgpack::unpack(pkt, pkt_len);
    msgpack::object obj = oh.get();

    // intialize actuators
    for (int i = 0; i < lbbNumOfPositionActuatorIds; ++i)
        actuators[i] = sensors[i * 3];

    // find robot name
    for (const auto &[k, val] : obj.via.map) {
        const auto &key{to_string(k)};
        if (key != "RobotConfig")
            continue;
        const auto &[bodyId, bodyVersion, headId, headVersion] = val.as<std::array<std::string, 4>>();
        robotName = DEFS::headid2name(headId);

        // update SHM info
        m_shm.updateRobotInfo(robotName, bodyId, bodyVersion, headId, headVersion);
        serial.body = bodyId;
        serial.head = headId;
        simulator = (robotName == RobotName::SIMULATOR);

        std::cout << std::endl;
        std::cout << "LoLa connection established on robot \xc2\xbb" << DEFS::enum2botname(robotName) << "\xc2\xab"
                  << std::endl;
        std::cout << "Head serial: " << serial.head << std::endl;
        std::cout << "Body serial: " << serial.body << std::endl;
        std::cout << std::endl;
        break;
    }

    while (running) {
        pkt_len = sock.receive(boost::asio::buffer(pkt, length), 0, ec);
        if (ec) {
            std::cerr << __PRETTY_FUNCTION__ << " - socket error, reconnecting: " << ec.message() << std::endl;
            connect();
            continue;
        }

        timestamp = getSystemTimestamp();
        if (pkt_len < LOLA_PKT_SIZE) {
            std::cerr << __PRETTY_FUNCTION__ << " - received incomplete LoLa msgpack object" << std::endl;
            continue;
        }

        // create msgpack object & parse sensor values
        msgpack::object_handle oh = msgpack::unpack(pkt, pkt_len);
        readSensors(oh.get());

        // generate msgpack object & write to socket
        const auto &buf{writeActuators()};
        sock.send(boost::asio::buffer(buf.data(), buf.size()), 0, ec);
        if (ec) {
            std::cerr << __PRETTY_FUNCTION__ << " - socket error, reconnecting: " << ec.message() << std::endl;
            connect();
            continue;
        }

        // update monitor SHM values
        m_shm.updateSensors(sensors);

        // check chest button
        buttonHandler();

        static int c{0};
        if (shutdown && sitDone) {
            // delay shutdown so LED requests will still be sent
            if (++c < 2)
                continue;
            // frontend has stopped & bot is sitting, save to power off
            say("Poweroff.");
            if (!simulator)
                system("/bin/systemctl poweroff");
            running = false;
        }

        // power off when on battery & charge is less than 10%
        if (sensors[batteryCurrentSensor] < 0) {
            if (sensors[batteryChargeSensor] < 0.25f) {
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

            if (sensors[batteryChargeSensor] < 0.1f) {
                say("Low battery!");
                toggleFrontend(false);
                shutdown = true;
            }
        }

        // kill zombies
        waitpid((pid_t)-1, 0, WNOHANG);
    }
}

void Connector::stop() {
    running = false;
}

void Connector::calibrateGyro() {
    using namespace Eigen;

    static constexpr int CALIB_COUNT{200};
    static Vector3f prevGyro, gyroOffset, gyroGain;
    static int count;

    if (simulator)
        return; // don't apply any calibration in webots simulator

    if (gyroGain == Vector3f(0, 0, 0)) {
        if (calibration::body.count(serial.body)) {
            gyroGain = calibration::body.at(serial.body).gyroGain;
            std::cerr << "Using gyro gain calibration: [" << gyroGain[0] << ", " << gyroGain[1] << ", " << gyroGain[2]
                      << "]" << std::endl;
        } else {
            gyroGain = {1, 1, 1};
            std::cerr << "ERROR: No gyro calibration found for " << serial.body << std::endl;
        }
    }

    Vector3f g = {sensors[gyroXSensor], sensors[gyroYSensor], sensors[gyroZSensor]};
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
    sensors[accYSensor] *= -1;
    g[2] *= -1;

    sensors[gyroXSensor] = g[0] * gyroGain[0];
    sensors[gyroYSensor] = g[1] * gyroGain[1];
    sensors[gyroZSensor] = g[2] * gyroGain[2];
}

void Connector::toggleFrontend(const bool start) {
    if (start) {
        if (forkexec("/home/nao/bin/jsfrontend")) {
            say("Starting frontend.");
        } else {
            std::cerr << __PRETTY_FUNCTION__ << " - fork() failed, cannot start frontend" << std::endl;
            say("Failed to start frontend.");
        }
    } else {
        forkexec("/usr/bin/killall", "jsfrontend");
        say("Stopping frontend.");
    }
}

void Connector::buttonHandler() {
    switch (checkBtnEvent(sensors[chestButtonSensor])) {
        case BtnClk::SINGLE:
            if (!frontendConnected)
                toggleFrontend(true);
            break;
        case BtnClk::TRIPPLE:
            toggleFrontend(!frontendConnected);
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
            system("/usr/libexec/reset-cameras.sh toggle");
        default:
            break;
    }
}

void Connector::readSensors(const msgpack::object &obj) {
    static int tick{0};
    // get reference to write buffer
    BBSensorData &bbsd{bb_shm->sensors.consumedData()};

    bbsd.tick = tick++;
    bbsd.robotName = robotName;
    bbsd.backendType = BackendType_Lola;
    bbsd.backendVersion = CURRENT_BACKEND_VERSION;
    bbsd.connected = frontendConnected;
    bbsd.staticMotionActive = false;
    bbsd.timestamp = timestamp;

    for (const auto &x : obj.via.map) {
        // enumerate
        const auto &key{to_string(x.key)};
        const auto &map{lola::mapping::sensors.at(key)};

        assert(map.size() == x.val.via.array.size);

        int i{-1};
        for (const auto &v : x.val.via.array) {
            ++i;
            if (map[i] < 0) // value ignored
                continue;
            sensors[map[i]] = v.as<float>();
        }
    }

    calibrateGyro();

    // emulate FSR total sensors
    sensors[lFSRTotalSensor] = sensors[lFSRFrontLeftSensor] + sensors[lFSRFrontRightSensor] +
                               sensors[lFSRRearLeftSensor] + sensors[lFSRRearRightSensor];
    sensors[rFSRTotalSensor] = sensors[rFSRFrontLeftSensor] + sensors[rFSRFrontRightSensor] +
                               sensors[rFSRRearLeftSensor] + sensors[rFSRRearRightSensor];

    std::copy(sensors.begin(), sensors.end(), std::begin(bbsd.sensors));

    // swap SHM buffers
    bb_shm->sensors.consume();

    //std::cerr << std::endl << obj << std::endl;
}

void Connector::setIdleLeds() {
    static const std::vector<float> gamma{0, 0.01, 0.05, 0.15, 0.3, 0.45, 0.6, 0.8, 1, 1};
    static int c{0}, idx{0}, inc{1};

    for (size_t i = headLedRearLeft0Actuator; i <= headLedMiddleLeft0Actuator; ++i)
        actuators[i] = gamma[idx];

    // blink brain leds as soon as gyro is calibrated
    if (c > 9 && calibrated) {
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

    // ears: used as battery indicator
    for (int i = 0; i < 10; ++i) {
        actuators[earsLedLeft0DegActuator + i] = actuators[earsLedRight324DegActuator - i] =
                (sensors[batteryChargeSensor] > (i - 1) / 10.f) * !shutdown;
    }

    // chest button: green when charging, off on battery
    if (sensors[batteryCurrentSensor] > 0) {
        actuators[chestBoardLedRedActuator] = actuators[chestBoardLedBlueActuator] = 0;
        actuators[chestBoardLedGreenActuator] = 0.25;
    } else {
        actuators[chestBoardLedRedActuator] = actuators[chestBoardLedGreenActuator] =
                actuators[chestBoardLedBlueActuator] = 0;
    }

    // eyes: white when running, off when shutdown has been initiated
    for (size_t i = faceLedRedLeft0DegActuator; i <= faceLedBlueRight315DegActuator; ++i)
        actuators[i] = !shutdown;
}

msgpack::sbuffer &Connector::writeActuators() {
    // get reference to write buffer
    bb_shm->actuators.timedProduce(10);
    const BBActuatorData &bbad{bb_shm->actuators.producedData()};

    // check if frontend has increased actuator tick
    static int last_tick{-1}, connCnt{0};
    if (bbad.tick != last_tick) {
        last_tick = bbad.tick;
        connCnt = std::min(connCnt + 1, 15);
    } else {
        connCnt = std::max(connCnt - 1, 0);
    }

    // assume frontend is connected after 10 ticks
    if (!frontendConnected && (connCnt > 10)) {
        frontendConnected = true;
        std::cout << "New frontend connection established." << std::endl;
    } else if (frontendConnected && (connCnt == 0)) {
        frontendConnected = false;
        std::cout << "Lost frontend connection." << std::endl;
    }

    if (frontendConnected) {
        sitDone = false;

        // copy to local buffer
        std::copy(std::begin(bbad.actuators), std::begin(bbad.actuators) + lbbNumOfActuatorIds, actuators.begin());

        // normalize LED values to [0..1] instead of [0..255] expected by NaoQi
        for (int i = faceLedRedLeft0DegActuator; i <= rFootLedBlueActuator; ++i)
            actuators[i] /= 255.f;
    } else { // no frontend connected
        // set status LEDs
        setIdleLeds();

        // check if torso is upright
        if (std::max(std::abs(sensors[angleXSensor]), std::abs(sensors[angleYSensor])) < 1) {
            // move to sit pose
            if (!sitDone)
                sitDone = sit(sensors.data(), actuators.data());
        } else {
            // unstiff all joints
            stiffness(-1);
            sitDone = false;
        }
    }

    // create msgpack buffer
    static msgpack::sbuffer buffer;
    buffer.clear();
    msgpack::packer<msgpack::sbuffer> pk(&buffer);

    pk.pack_map(lola::mapping::actuators.size());

    static const std::string SONAR{"Sonar"};
    for (const auto &[chain, values] : lola::mapping::actuators) {
        // sonar need be handled separately, since it's bool, not float
        if (chain == SONAR)
            continue;

        pk.pack(chain);
        pk.pack_array(values.size());
        for (const auto &i : values)
            pk.pack(actuators[i]);
    }

    // add sonar actuators
    pk.pack(SONAR);
    const auto &s{lola::mapping::actuators.at(SONAR)};
    pk.pack_array(s.size());
    for (const auto &i : s)
        pk.pack(actuators[i] > 0); // lola only has on/off for sonar

    //std::cerr << std::endl << msgpack::unpack(buffer.data(), buffer.size()).get() << std::endl;
    return buffer;
}

void Connector::say(const std::string &text) {
    static TTSFifo tts;

    std::cout << "[TTS] " << text << std::endl;
    tts.say(text);
}

void Connector::stiffness(const float &v) {
    for (const auto &i : mapping::sensors.at("Stiffness"))
        actuators[i] = v;
}

} // namespace lola

// vim: set ts=4 sw=4 sts=4 expandtab:
