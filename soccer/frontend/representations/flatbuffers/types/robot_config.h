#pragma once

#include <string_view>

namespace bbipc {
struct HardwareId {
    std::string_view serial, version;
};

struct RobotConfig {
    HardwareId head, body;
};
} // namespace bbipc
