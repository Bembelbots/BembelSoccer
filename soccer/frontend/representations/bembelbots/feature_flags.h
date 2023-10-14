#pragma once

#include <framework/rt/flags.h>

enum class FeatureFlag : uint32_t {
    //EnableLogging = rt::Flag::CustomOffset << 0,
};

static constexpr rt::Flag operator|(rt::Flag f1, FeatureFlag f2) {
    return static_cast<rt::Flag>(
            static_cast<uint32_t>(f1) | static_cast<uint32_t>(f2));
}

static constexpr rt::Flag operator&(rt::Flag f1, FeatureFlag f2) {
    return static_cast<rt::Flag>(
            static_cast<uint32_t>(f1) & static_cast<uint32_t>(f2));
}
