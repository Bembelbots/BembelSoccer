#include "botnames.h"
#include <stdexcept>
#include <string_view>
#include <unordered_map>

#include <botnames_generated.h>
#include <framework/util/assert.h>

using namespace std;

std::unordered_map<string_view, RobotName> serials{
    {"P0000000000000000000", RobotName::SIMULATOR}
};

string str_tolower(string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

RobotName DEFS::headid2name(const string_view &id) {
    if (serials.count(id) > 0)
        return serials.at(id);
    return RobotName::UNKNOWN;
}

string DEFS::enum2botname(const RobotName &n) {
    std::string name(bbapi::EnumNameRobotName(n));
    jsassert(!name.empty()) << __PRETTY_FUNCTION__ << ": invalid RoboName value " << int(n);
    return name;
}

string DEFS::enum2hostname(const RobotName &n) {
    return str_tolower(DEFS::enum2botname(n));
}
