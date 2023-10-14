#include "util.h"

namespace  rt {

std::string composeError(std::string_view prefix, std::string_view errorMsg) {
    // TODO: Use c++20 std::format for error messages to make composing easier
    return std::string("  - ") + std::string(prefix) + ": " + std::string(errorMsg) + "\n";
}

} // namespace rt
