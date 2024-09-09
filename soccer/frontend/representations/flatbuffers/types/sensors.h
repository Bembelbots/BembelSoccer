#pragma once
#include <botnames_generated.h>
#include <sensors_generated.h>

#include <array>
#include <string_view>
#include <Eigen/Core>
#include <Eigen/src/Core/Matrix.h>

#include <representations/flatbuffers/types/lola_names.h>

namespace bbipc {
using JointArray = std::array<float, LOLA_NUMBER_OF_JOINTS>;
using JointStatus = std::array<int, LOLA_NUMBER_OF_JOINTS>;

template<typename T, size_t N, uint16_t M>
void copyArray(std::array<T, N> &to, const ::flatbuffers::Array<T, M> *from) {
    static_assert(M == N);
    to = *reinterpret_cast<const std::array<T, N> *>(from);
}

struct HardwareId {
    std::string_view serial, version;

    bool operator==(const HardwareId &other) const { return serial == other.serial && version == other.version; }
};

struct RobotConfig {
    HardwareId head, body;

    bool operator==(const RobotConfig &other) const { return head == other.head && body == other.body; }
};

struct JointSensors {
    JointArray position, stiffness, temperature, current;
    JointStatus status;

    bool operator==(const JointSensors &other) const {
        return position == other.position && stiffness == other.stiffness && status == other.status &&
               temperature == other.temperature && current == other.current;
    }
};

struct Battery {
    float charge, current, status, temperature;

    bool operator==(const Battery &other) const {
        return charge == other.charge && current == other.current && status == other.status &&
               temperature == other.temperature;
    }

    bool isCharging() const { return !(static_cast<int>(status) & 0b100000); }
};

struct SonarSensors {
    float left, right;

    bool operator==(const SonarSensors &other) const { return left == other.left && right == other.right; }
};

struct IMU {
    Eigen::Vector3f accelerometer, gyroscope;
    Eigen::Vector2f angles;

    bool operator==(const IMU &other) const {
        return accelerometer == other.accelerometer && gyroscope == other.gyroscope && angles == other.angles;
    }
};

struct FSRFoot {
    using FSRFootArray = std::array<float, 4>;

    union {
        struct {
            float fl, fr, rl, rr;
        };
        FSRFootArray arr;
    };

    FSRFoot() = default;
    FSRFoot(const float _fl, const float _fr, const float _rl, const float _rr) : fl(_fl), fr(_fr), rl(_rl), rr(_rr) {}
    explicit FSRFoot(const FSRFootArray &fa) : arr(fa) {}

    bool operator==(const FSRFoot &other) const { return arr == other.arr; }
};

struct FSR {
    FSRFoot left, right;

    FSR() = default;
    FSR(const FSRFoot &l, const FSRFoot &r) : left(l), right(r) {}
    bool operator==(const FSR &other) const { return left == other.left && right == other.right; }
};

struct TouchChest {
    float button;

    bool operator==(const TouchChest &other) const { return button == other.button; }
};

struct TouchHead {
    float front, middle, rear;

    bool operator==(const TouchHead &other) const {
        return front == other.front && middle == other.middle && rear == other.rear;
    }
};

struct TouchHand {
    float back, left, right;

    bool operator==(const TouchHand &other) const {
        return back == other.back && left == other.left && right == other.right;
    }
};

struct TouchHands {
    TouchHand left, right;

    bool operator==(const TouchHands &other) const { return left == other.left && right == other.right; }
};

struct TouchFoot {
    float bumper_left, bumper_right;

    bool operator==(const TouchFoot &other) const {
        return bumper_left == other.bumper_left && bumper_right == other.bumper_right;
    }
};

struct TouchFeet {
    TouchFoot left, right;

    bool operator==(const TouchFeet &other) const { return left == other.left && right == other.right; }
};

struct Touch {
    TouchChest chest;
    TouchHead head;
    TouchHands hands;
    TouchFeet feet;

    bool operator==(const Touch &other) const {
        return chest == other.chest && head == other.head && hands == other.hands && feet == other.feet;
    }

    using TouchArray = std::array<float, 14>;
    TouchArray arr() const {
        return {chest.button,
                head.front,
                head.middle,
                head.rear,
                feet.left.bumper_left,
                feet.left.bumper_right,
                hands.left.back,
                hands.left.left,
                hands.left.right,
                feet.right.bumper_left,
                feet.right.bumper_right,
                hands.right.back,
                hands.right.left,
                hands.right.right};
    }
};

struct Sensors {
    Battery battery;
    JointSensors joints;
    FSR fsr;
    IMU imu;
    SonarSensors sonar;
    Touch touch;

    bool operator==(const Sensors &other) const {
        return battery == other.battery && joints == other.joints && fsr == other.fsr && imu == other.imu &&
               sonar == other.sonar && touch == other.touch;
    }
};

struct SensorMsg {
    bbapi::RobotName robotName;
    int32_t tick;
    int64_t timestamp;
    bool connected;
    bool simulator;
    uint8_t backendVersion;

    RobotConfig robotConfig;
    Sensors sensors;
};
} // namespace bbipc

namespace flatbuffers {
bbapi::Sensors Pack(const bbipc::Sensors &s);
bbipc::Sensors UnPack(const bbapi::Sensors &s);
} // namespace flatbuffers
