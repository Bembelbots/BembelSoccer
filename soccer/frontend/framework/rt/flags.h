#pragma once
#include <cstdint>
#include <stdexcept>

namespace  rt {

enum class Flag : uint32_t {
    None            = 1 << 0,
    OptionalInput   = 1 << 1,
    RequiredInput   = 1 << 2,
    Output          = 1 << 3,
    Event           = 1 << 4,
    EnableLogging   = 1 << 5,
    DisableLogging  = 1 << 6,
    ReservedEnd     = Event,
    CustomOffset    = (ReservedEnd << 1),
};

struct NegativeFlag {
    Flag flag;
};

static constexpr Flag None              = Flag::None;
static constexpr Flag OptionalInput     = Flag::OptionalInput;
static constexpr Flag OutputFlag        = Flag::Output;
static constexpr Flag Require           = Flag::RequiredInput;
static constexpr Flag Event             = Flag::Event;
static constexpr Flag EnableLogging     = Flag::EnableLogging;
static constexpr Flag DisableLogging    = Flag::DisableLogging;

constexpr uint32_t flagToInt(Flag flag) {
    return static_cast<uint32_t>(flag);
}

constexpr uint32_t flagToInt(NegativeFlag nf) {
    return static_cast<uint32_t>(nf.flag);
}

constexpr Flag operator|(Flag f1, Flag f2) {
    return static_cast<Flag>(flagToInt(f1) | flagToInt(f2));
}

constexpr Flag operator&(Flag f1, Flag f2) {
    return static_cast<Flag>(flagToInt(f1) & flagToInt(f2));
}

constexpr Flag operator&(Flag f1, NegativeFlag f2) {
    return static_cast<Flag>(flagToInt(f1) & ~flagToInt(f2));
}

constexpr NegativeFlag operator~(Flag f) {
    return { .flag = f };
}

constexpr Flag& operator|=(Flag &f1, Flag f2) {
    f1 = f1 | f2;
    return f1;
}

constexpr Flag& operator&=(Flag &f1, Flag f2) {
    f1 = f1 & f2;
    return f1;
}

constexpr uint32_t operator<<(Flag flag, uint8_t shift) {
    return flagToInt(flag) << shift;
}

constexpr uint32_t operator>>(Flag flag, uint8_t shift) {
    return flagToInt(flag) >> shift;
}

constexpr bool any(Flag flag) {
    return static_cast<bool>(flag);
}

constexpr bool flag_set(Flag flags, Flag f) {
    return any(flags & f);
}

} // namespace rt
