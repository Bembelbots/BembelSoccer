#pragma once

#include <string_view>
#include <framework/serialize/serializer.h>

/*  Enum          , name            , hostname        , head_id */
#define BOT_LIST \
    BOT(UNKNOWN       , "unknown"       , "unknown"       , "") \
    BOT(SIMULATOR     , "Simulator"     , "simulator"     , "P0000000000000000000")


//Robot names
enum class RobotName {
#define BOT(sym, name, host, head_id) sym,
    BOT_LIST
#undef BOT
};


namespace DEFS {
// convert head ID to name enum
RobotName headid2name(const std::string_view &id);

// convert enum to "human readable" string (use this for text2speech)
RobotName botname2enum(const std::string_view &n);

// convert enum to hostname (no capitalization, no spaces)
RobotName hostname2enum(const std::string_view &n);

// reverse lookup for string botname
std::string_view enum2botname(const RobotName &n);

// reverse lookup for string hostname
std::string_view enum2hostname(const RobotName &n);

// reverse lookup for string headid
std::string_view enum2headid(const RobotName &n);
}

SERIALIZE(RobotName, {
    return {key, std::string(DEFS::enum2botname(value))};
});

// vim: set ts=4 sw=4 sts=4 expandtab:
