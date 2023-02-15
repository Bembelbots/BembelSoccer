#pragma once

#include "introspection.h"
#include "snapshot.h"
#include <set>
#include <mutex>
#include "../util/assert.h"

class ConfigParser;

struct ConfigFiles {
    ConfigParser *settings;
    ConfigParser *calib;
};


class BlackboardBase : public Introspection {
public:
    explicit BlackboardBase(SnapshotTag &&)
        : Introspection(), context(std::make_shared<BlackboardContext>()) {

    }
    explicit BlackboardBase(const std::string &name);
    virtual ~BlackboardBase();

    std::string getBlackboardName() const;

    virtual bool loadConfig(ConfigFiles &) { return true; }
    virtual bool writeConfig(ConfigFiles &) { return true; }

    [[nodiscard]] std::scoped_lock<std::mutex> scopedLock() {
        return std::scoped_lock<std::mutex>(context->mtx);
    }

private:
    struct BlackboardContext {
        std::mutex mtx;
    };

    std::string _myName;
    std::shared_ptr<BlackboardContext> context;
protected:
    bool registered = false;
};

class Blackboard : public BlackboardBase {
public:
    using BlackboardBase::BlackboardBase;
    virtual ~Blackboard() = default;
};

class SnapshotableBlackboard : public BlackboardBase {
public:
    using BlackboardBase::BlackboardBase;
    virtual ~SnapshotableBlackboard() = default;
    virtual void setup() = 0;
};

// vim: set ts=4 sw=4 sts=4 expandtab
