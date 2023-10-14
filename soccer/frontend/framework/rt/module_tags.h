#pragma once
#include <cstdint>
#include <stdexcept>

namespace  rt {

enum class ModuleTag : uint32_t {
    None            = 0,
    Normal          = 1 << 1,
    NoThread        = 1 << 2,
    Hook            = 1 << 3,
    Logger          = 1 << 4,
    DisableLogging  = 1 << 5,
};

constexpr uint32_t tagToInt(ModuleTag tag) {
    return static_cast<uint32_t>(tag);
}

constexpr ModuleTag operator|(ModuleTag t1, ModuleTag t2) {
    return static_cast<ModuleTag>(tagToInt(t1) | tagToInt(t2));
}

constexpr ModuleTag operator&(ModuleTag &t1, ModuleTag t2) {
    return static_cast<ModuleTag>(tagToInt(t1) & tagToInt(t2));
}

constexpr ModuleTag& operator|=(ModuleTag &t1, ModuleTag t2) {
    t1 = t1 | t2;
    return t1;
}

constexpr ModuleTag& operator&=(ModuleTag &t1, ModuleTag t2) {
    t1 = t1 & t2;
    return t1;
}

constexpr bool any(ModuleTag tag) {
    return static_cast<bool>(tag);
}

constexpr bool tag_set(ModuleTag tags, ModuleTag t) {
    return any(tags & t);
}

} // namespace rt
