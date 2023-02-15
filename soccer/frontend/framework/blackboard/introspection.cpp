#include "introspection.h"

#include "../logger/logger.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace boost::property_tree;

Introspection::Introspection() :
    _varPrefix("snapshot")
{ }

Introspection::Introspection(const std::string &varPrefix) :
    _varPrefix(varPrefix)
{ }

void Introspection::setDebugSymbol(const std::string &name,
                                   const bool &value) {
    if (_debugValues.count(name) == 0) {
#ifndef STANDALONE_BEHAVIOR
        LOG_WARN << "key " << name << " not found in debug map!";
#endif
        return;
    }
    *_debugValues[name] = value;
}

std::vector<std::string> Introspection::getDebugSymbols() const {
    return _debugVariables;
}

std::vector<bool> Introspection::getDebugValues() {
    std::vector<std::string> symbols = getDebugSymbols();
    std::vector<bool> r;
    for (size_t i = 0; i < symbols.size(); ++i) {
        r.push_back(*_debugValues[symbols[i]]);
    }
    return r;
}

void Introspection::updateDebugSymbols(const std::vector< std::pair<std::string, std::string> > &pairs) {
    for (const auto &p: pairs) {
#ifndef STANDALONE_BEHAVIOR
        LOG_INFO << "set symbol " << p.first << " to " << p.second;
#endif
        try {
            auto &val = _debugValues.at(p.first);
            *val = (p.second == std::string("1"));
        } catch(...) {
            LOG_ERROR << "failed to set symbol " << p.first << " to " << p.second;
        }
    }
}

bool Introspection::isEditable(const std::string &key) {
    auto it = _set_funcs.find(key);
    return it != _set_funcs.end();
}

string Introspection::toJson() const {
    ptree asPt;
    for (const auto &val : _get_funcs) {
        BlackboardDataContainer con = val.second();
        asPt.put(con.getKey(), con.getValue());
    }
    stringstream ss;
    json_parser::write_json(ss, asPt);
    return ss.str();
}

void Introspection::addSwitch(bool *dbg, const std::string &name, const std::string &desc) {
    *dbg = false;
    _debugVariables.push_back(name);
    _debugDescriptions.insert(tStringPair(name, std::string(desc)));
    _debugValues.insert(tStringBoolPair(name, dbg));
}

// vim: set ts=4 sw=4 sts=4 expandtab:
