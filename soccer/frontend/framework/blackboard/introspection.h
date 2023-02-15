#pragma once

#include "datacontainer.h"

#include <functional>
#include <string>
#include <vector>
#include <map>
#include <sstream>

typedef std::pair<std::string, std::string> tStringPair;
typedef std::vector<tStringPair> tStringPairList;
typedef std::pair<std::string, bool *> tStringBoolPair;

// macro to create new class variable which CAN be debugable
// later on. this creates a variable with name varname and from
// type t. additionaly another bool variable is created, which
// is used to enable / disable debug in the serialization part.

#define MAKE_VAR(t, varname) \
    t varname; \
    bool varname ## _debug

#define _INIT_DEBUGGABLE(varname, val, description) \
    varname = val

#define INIT_VAR(varname, val, description) \
    _INIT_DEBUGGABLE(varname, val, description); \
    addVariable(&varname, &varname ## _debug, #varname, description)
    /* _get_funcs[#varname] = getter_fabric(_varPrefix + "." + #varname, &varname) */

#define INIT_VAR_RW(varname, val, description) \
    _INIT_DEBUGGABLE(varname, val, description); \
    addVariableRW(&varname, &varname ## _debug, #varname, description)

// macros for enums
// reinterpret them as ints to avoid base64 encode & setter_fabric issues
#define INIT_ENUM(varname, val, description) \
    _INIT_DEBUGGABLE(varname, val, description); \
    addVariable<int>(reinterpret_cast<int *>(&varname), &varname ## _debug, #varname, description)

#define INIT_ENUM_RW(varname, val, description) \
    _INIT_DEBUGGABLE(varname, val, description); \
    addVariableRW<int>(reinterpret_cast<int *>(&varname), &varname ## _debug, #varname, description)

// use for on/off switches (e.g. cam images),
// does return debug state instead of actual variable in naodebug
#define INIT_SWITCH(varname, val, description) \
    _INIT_DEBUGGABLE(varname, val, description); \
    addSwitch(&varname ## _debug, #varname, description)

// check if a variable is being debugged by bembelDbug
#define DEBUG_ON(varname) varname ## _debug

class BlackboardDataContainer;
class Introspection {

public:

    typedef std::function<void(const std::string &s)> tBBSetFunc;
    typedef std::function<BlackboardDataContainer(void)> tBBGetFunc;

    std::vector<std::string> _debugVariables;
    std::map<std::string, std::string> _debugDescriptions;
    std::map<std::string, bool *> _debugValues;
    std::map<std::string, tBBSetFunc> _set_funcs;
    std::map<std::string, tBBGetFunc> _get_funcs;
    
    explicit Introspection();
    explicit Introspection(const std::string &varPrefix);

    // get symbols and values
    std::vector<std::string> getDebugSymbols() const;
    std::vector<bool> getDebugValues();

    // set value(s) and update symbol state (broadcast: yes/no)
    void setDebugSymbol(const std::string &name, const bool &value);
    void updateDebugSymbols(const tStringPairList &pairs);

    bool isEditable(const std::string &);

    //Convert the child-class to json.
    std::string toJson() const;

protected:

    std::string _varPrefix;

    void addSwitch(bool *dbg, const std::string &name, const std::string &desc);

    template<typename T>
    void addVariable(T *var, bool *dbg, const std::string &name, const std::string &desc) {
        addSwitch(dbg, name, desc);
        _get_funcs[name] = getter_fabric(_varPrefix + "." + name, var);
    }

    template<typename T>
    void addVariableRW(T *var, bool *dbg, const std::string &name, const std::string &desc) {
        addVariable(var, dbg, name, desc);
        _set_funcs[name] = setter_fabric(var);
    }

    //////  used inside the INIT_VAR-macro:
    // when called, returns a function to ecapsulate the passed 'target'
    // inside a "closure", a 'setter' (naodebug) actually calls the inner func
    // and provides 's', which is den parsed and assigned.
    template<typename T>
    static std::function<void(const std::string &)> setter_fabric(T *target) {
        return [=](const std::string& s) -> void {
            std::stringstream ss;
            ss << s;
            ss >> *target;
        };
    }

    // the same pattern for the getter, the fabric returns a 'customized'
    // function, which delivers the containing value as 'string', if being asked for
    template <typename T>
    static std::function<BlackboardDataContainer(void)> getter_fabric(
        const std::string &key, const T *source) {
        return [=](void) -> BlackboardDataContainer { return {key, *source}; };
    }
};

// vim: set ts=4 sw=4 sts=4 expandtab:
