#pragma once

#include <boost/property_tree/ptree.hpp>
#include <exception>
#include <map>
#include <string>

#define READ_KEY(cfg, key, type) key = cfg->get<type>(#key)
#define READ_KEY_ENUM(cfg, key, type) key = static_cast<type>(cfg->get<int>(#key))
#define READ_KEY_SUBSECTION(cfg, subsection, key, type) key = cfg->get<type>(subsection + "." + #key)

// try reading key, ignoring error (key will not be modified on failure)
#define READ_KEY_TRY(cfg, key, type) if (cfg->hasKey(#key)) key = cfg->get<type>(#key)
#define READ_KEY_SUBSECTION_TRY(cfg, subsection, key, type) if (cfg->hasKey(subsection, #key)) key = cfg->get<type>(subsection + "." + #key)

#define WRITE_KEY(cfg, key, type) WRITE_KEY_VALUE(cfg, key, key, type)
#define WRITE_KEY_ENUM(cfg, key) WRITE_KEY_VALUE(cfg, key, static_cast<int>(key), int)
#define WRITE_KEY_VALUE(cfg, key, value, type) cfg->put<type>(#key, value)

#define WRITE_KEY_SUBSECTION(cfg, subsection, key, type) cfg->put<type>(subsection + "." + #key, key)


class ConfigParser {
public:
    ConfigParser(const std::string &filename, const std::string_view section);

    template<typename T>
    T get(const std::string &param) {
        try {
            return (_pt.get<T>(_section + "." + param));
        } catch (boost::property_tree::ptree_bad_path &e) {
            throw (std::runtime_error(std::string("node_unknown: ") + e.what()));
        } catch (boost::property_tree::ptree_bad_data &e) {
            throw (std::runtime_error(std::string("typeconversion not possible: ") +
                                      e.what()));
        } catch (std::exception &e) {
            throw (std::runtime_error(std::string("should not happen: ") + e.what()));
        }
    }

    // use put
    template <typename T>
    void put(const std::string &param, const T &value) {
        _pt.put(_section + "." + param, value);
    }

    void rename_section(const std::string &section, const std::string &new_section) {
        _pt.put_child(new_section, _pt.get_child(section));
        _pt.erase(section);
        _section = new_section;
    }

    bool hasKey(const std::string &key);
    bool hasKey(const std::string &subsection, const std::string &key);

    // map<string, string>, contains all data
    typedef std::map<std::string, std::string> cfgPair;
    cfgPair params();

    // loads a std::string in _pt, and if it doesn´t fail updates the file
    // what does this load and update? i don't understand...
    ConfigParser *load(const std::string &ini_as_string);

    bool save();
    bool save(const std::string &filename);

    std::string getMyCfgAsString();

    // update a subtree of whole config file.
    bool replaceSubtree(const std::string &section, const std::string &subtree);

private:
    std::string _configFilename;
    std::string _section;
    boost::property_tree::ptree _pt;
    void init();
    void read(const std::string &fName);
    void read(std::stringstream &ss);

};

std::ostream &operator<<(std::ostream &os, ConfigParser &cfg);

// vim: set ts=4 sw=4 sts=4 expandtab:
