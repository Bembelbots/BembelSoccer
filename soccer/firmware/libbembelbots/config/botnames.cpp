#include "botnames.h"
#include <map>

using namespace std;

RobotName tryLookupEnum(const map<string_view, RobotName> &str2enum,
                        const string_view &id) {
    auto it = str2enum.find(id);
    if (it != str2enum.end()) {
        return it->second;
    }
    return RobotName::UNKNOWN;
}

RobotName DEFS::headid2name(const string_view &id) {
    map<string_view, RobotName> _head2name = {
#define BOT(sym, name, host, head_id) {head_id, RobotName::sym},
        BOT_LIST
#undef BOT
    };
    return tryLookupEnum(_head2name, id);
}

RobotName DEFS::botname2enum(const string_view &n) {
    map<string_view, RobotName> _name2enum = {
#define BOT(sym, name, host, head_id) {name, RobotName::sym},
        BOT_LIST
#undef BOT
    };
    return tryLookupEnum(_name2enum, n);
}

RobotName DEFS::hostname2enum(const string_view &n) {
    map<string_view, RobotName> _host2enum = {
#define BOT(sym, name, host, head_id) {host, RobotName::sym},
        BOT_LIST
#undef BOT
    };
    return tryLookupEnum(_host2enum, n);
}

string_view DEFS::enum2botname(const RobotName &n) {
    map<RobotName, string_view> _enum2name = {
#define BOT(sym, name, host, head_id) {RobotName::sym, name},
        BOT_LIST
#undef BOT
    };
    return _enum2name.at(n);
}

string_view DEFS::enum2hostname(const RobotName &n) {
    map<RobotName, string_view> _enum2host = {
#define BOT(sym, name, host, head_id) {RobotName::sym, host},
        BOT_LIST
#undef BOT
    };
    return _enum2host.at(n);
}

string_view DEFS::enum2headid(const RobotName &n) {
    map<RobotName, string_view> _enum2head = {
#define BOT(sym, name, host, head_id) {RobotName::sym, head_id},
        BOT_LIST
#undef BOT
    };
    return _enum2head.at(n);
}

// vim: set ts=4 sw=4 sts=4 expandtab:
