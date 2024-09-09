#pragma once

#include <array>
#include <vector>

#include "actuators_generated.h"
#include "sensors.h"

namespace bbipc {
struct JointActuators {
    JointArray position, stiffness;

    JointActuators() {
        position.fill(0);
        stiffness.fill(0);
    }

    bool operator==(const JointActuators &other) const {
        return position == other.position && stiffness == other.stiffness;
    }
};

using LEDEar = std::array<float, 10>;
struct LEDEars {
    LEDEar left, right;

    LEDEars() {
        left.fill(0);
        right.fill(0);
    }

    bool operator==(const LEDEars &other) const { return left == other.left && right == other.right; }
};

template<size_t N>
struct LEDString {
    static constexpr size_t NUM_LEDS{N};
    std::array<float, N> r;
    std::array<float, N> g;
    std::array<float, N> b;

    LEDString() { fill(0, 0, 0); }

    bool operator==(const LEDString &other) const { return r == other.r && g == other.g && b == other.b; }

    void fill(const float v) { fill(v, v, v); }
    void fill(const float _r, const float _g, const float _b) {
        r.fill(_r);
        g.fill(_g);
        b.fill(_b);
    }
};

struct LEDEyes {
    LEDString<8> left, right;

    bool operator==(const LEDEyes &other) const { return left == other.left && right == other.right; }
};

struct LEDSingle {
    float r{0}, g{0}, b{0};

    bool operator==(const LEDSingle &other) const { return r == other.r && g == other.g && b == other.b; }
};

struct LEDFeet {
    LEDSingle left, right;

    bool operator==(const LEDFeet &other) const { return left == other.left && right == other.right; }
};

using LEDSkull = std::array<float, 12>;

struct LED {
    LEDSingle chest;
    LEDEars ears;
    LEDEyes eyes;
    LEDFeet feet;
    LEDSkull skull;

    bool operator==(const LED &other) const {
        return chest == other.chest && ears == other.ears && eyes == other.eyes && feet == other.feet &&
               skull == other.skull;
    }
};

struct SonarActuators {
    bool left{false}, right{false};

    bool operator==(const SonarActuators &other) const { return left == other.left && right == other.right; }
};

struct Actuators {
    JointActuators joints;
    LED led;
    SonarActuators sonar;

    bool operator==(const Actuators &other) const {
        return joints == other.joints && led == other.led && sonar == other.sonar;
    }
};

struct ActuatorMsg {
    int32_t tick{0};
    uint64_t timestamp{0};
    Actuators actuators;
};
} // namespace bbipc

namespace flatbuffers {
bbapi::Actuators Pack(const bbipc::Actuators &act);
bbipc::Actuators UnPack(const bbapi::Actuators &act);
} // namespace flatbuffers

std::ostream &operator<<(std::ostream &s, const bbipc::Actuators &a);
