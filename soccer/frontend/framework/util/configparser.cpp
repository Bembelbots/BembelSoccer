#include "configparser.h"

#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace boost;


ConfigParser::ConfigParser(const string &filename, const string_view section)
    : _configFilename(filename)
    , _section(section) {
    init();
}

void ConfigParser::init() {
    try {
        read(_configFilename);
    } catch (property_tree::ptree_bad_data &e) {
        throw (runtime_error(string("Invalid config format: ") + e.what()));
    }
}

ConfigParser::cfgPair ConfigParser::params() {
    cfgPair res;

    for (const auto &item : _pt) {
        for (const auto &subitem : item.second) {
            string tmp = item.first + "." + subitem.first;
            res[tmp] = get<string>(tmp);
        }
    }

    return res;
}

ConfigParser *ConfigParser::load(const string &ini_as_string) {
    try {
        stringstream ss;
        ss << ini_as_string;
        read(ss);
    } catch (property_tree::ptree_bad_data &e) {
        throw (runtime_error(string("Invalid format: ") + e.what()));
    }

    return this;
}

bool ConfigParser::replaceSubtree(const std::string &section,
                                  const std::string &subtree) {
    boost::property_tree::ptree myTree;
    try {
        // read new stuff...
        stringstream ss;
        ss << subtree;
        property_tree::json_parser::read_json(ss, myTree);
    } catch (property_tree::ptree_bad_data &e) {
        throw (runtime_error(string("Invalid format: ") + e.what()));
    }

    // replace subtree,
    // errorhandling?
    _pt.put_child(section, myTree.get_child(section));

    return true;
}

bool ConfigParser::hasKey(const std::string &key) {
    try {
        get<std::string>(key);
        return true;
    } catch(...) {}
    return false;
}

bool ConfigParser::hasKey(const std::string &subsection, const std::string &key) {
    return hasKey(subsection + "." + key);
}

bool ConfigParser::save() {
    return save(_configFilename);
}

bool ConfigParser::save(const string &filename) {
    // i think this is missleading.
    // save should write the whole buffern,
    // but write_json just writes a key : pair to a buffer?

    std::stringstream ss;
    property_tree::json_parser::write_json(ss, _pt);


    ofstream file;
    file.open(filename.c_str());
    if (!file.is_open()) {
        return false;
    }
    file << ss.str();
    file.close();

    return true;

}

ostream &operator<<(ostream &os, ConfigParser &cfg) {
    // this one is buggy if we want to return calibration...
    ConfigParser::cfgPair map = cfg.params();
    for (const auto &item : map) {
        os << item.first << " = " << item.second << endl;
    }
    return os;
}

std::string ConfigParser::getMyCfgAsString() {
    std::stringstream ss;

    // create a local copy of the whole property tree
    // to simply return a tree with my configuration.
    // this is mainly used for broadcasting my own
    // configuration / calibration.
    boost::property_tree::ptree myTree;
    myTree.put_child(_section, _pt.get_child(_section));

    property_tree::json_parser::write_json(ss, myTree);
    return ss.str();
}

void ConfigParser::read(const string &fName) {
    property_tree::json_parser::read_json(fName, _pt);
}

void ConfigParser::read(stringstream &ss) {
    property_tree::json_parser::read_json(ss, _pt);
}

// vim: set ts=4 sw=4 sts=4 expandtab:
