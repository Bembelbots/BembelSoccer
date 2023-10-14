#pragma once

#include <string_view>
#include <framework/serialize/serializer.h>
#include <botnames_generated.h>

using bbapi::RobotName;

namespace DEFS {
// convert head ID to name enum
RobotName headid2name(const std::string_view &id);

// reverse lookup for string botname
std::string enum2botname(const RobotName &n);

// reverse lookup for string hostname
std::string enum2hostname(const RobotName &n);
}

SERIALIZE(RobotName, {
    return {key, std::string(DEFS::enum2botname(value))};
});

// vim: set ts=4 sw=4 sts=4 expandtab:
