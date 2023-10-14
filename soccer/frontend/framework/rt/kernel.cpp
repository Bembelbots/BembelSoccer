#include "kernel.h"

#include "depth_first_search.h"
#include "meta.h"

#include "../blackboard/blackboard.h"
#include "../util/assert.h"
#include "../thread/util.h"

#include <iostream>
#include <sstream>

using namespace rt;

ModuleId Kernel::addModule(Module *module, ModuleId expectedId) {
    jsassert(expectedId == modules.size());
    modules.emplace_back(module);
    return expectedId;
}

std::string Kernel::composeError(std::string_view prefix, std::string_view errorMsg) {
    // TODO: Use c++20 std::format for error messages to make composing easier
    return std::string("  - ") + std::string(prefix) + ": " + std::string(errorMsg) + "\n";
}

std::string Kernel::channelError(ChannelId id, std::string_view errorMsg) const {
    auto &chan = meta.channels.at(id);
    std::string messageName = prettyTypeName(chan.dataType);

    std::string messageType = "<<< UNKNOW MESSAGE TYPE >>>";
    switch (chan.kind) {
        case ChannelMeta::Type::BLOB:
        case ChannelMeta::Type::MESSAGE:
            messageType = "Message";
            break;
        case ChannelMeta::Type::COMMAND:
            messageType = "Commands of type";
            break;
        case ChannelMeta::Type::CONTEXT:
            messageType = "Context";
            break;
    }

    return composeError(messageType + " '" + messageName + "'", errorMsg);
}

std::string Kernel::moduleError(ModuleId id, std::string_view errorMsg) const {
    return composeError("Module '" + meta.modules.at(id).name + "'", errorMsg);
}

std::pair<bool, std::string> Kernel::resolve() {
    jsassert(modules.size() == meta.modules.size());
    jsassert(state == State::SETUP);

    // TODO check for errors in dependency graph here
    meta.setRequiredBy();

    int nErrors = 0;
    std::stringstream errorAcc;

    errorAcc << "Found the following errors in module dependencies:";

    // Look for circular dependencies
    DepthFirstSearch dfs;
    std::vector<CircularDep> circularDeps = dfs.findCycles(meta.modules);
    for (auto &cycle : circularDeps) {
        errorAcc << moduleError(
                cycle.first, "Circular dependency with module '" + meta.modules.at(cycle.second).name + "'.");
        nErrors++;
    }

    // Ensure messages have exactly one producer (& commands have only one handler)
    for (auto &chan : meta.channels) {
        switch (chan.kind) {
            case ChannelMeta::Type::BLOB:
            case ChannelMeta::Type::MESSAGE: {
                std::vector<ModuleId> producers;
                for (EndpointId &pointId : chan.endpoints) {
                    EndpointMeta &point = meta.endpoints.at(pointId);
                    if (point.kind == EndpointMeta::Direction::IN) {
                        continue;
                    }
                    producers.push_back(point.module);
                }
                if (producers.empty()) {
                    errorAcc << channelError(chan.id, "Message has no producer.");
                    nErrors++;
                }
                if (producers.size() >= 2) {
                    errorAcc << channelError(chan.id, "Message has more than one producer.");
                    nErrors++;
                }
                break;
            }
            case ChannelMeta::Type::COMMAND: {
                std::vector<ModuleId> handlers;
                for (auto &pointId : chan.endpoints) {
                    EndpointMeta &point = meta.endpoints.at(pointId);
                    if (point.kind == EndpointMeta::Direction::OUT) {
                        continue;
                    }
                    handlers.push_back(point.module);
                }
                if (handlers.empty()) {
                    errorAcc << channelError(chan.id, "Commands have no handler.");
                    nErrors++;
                }
                if (handlers.size() >= 2) {
                    errorAcc << channelError(chan.id, "Commands have multiple handlers.");
                    nErrors++;
                }
                break;
            }
            case ChannelMeta::Type::CONTEXT:
                break; // Nothing to check here yet
        }
    }

    bool ok = (nErrors == 0);
    state = ok ? State::READY : State::ERROR;
    std::string errorMsg = ok ? std::string{} : errorAcc.str();

    return {ok, errorMsg};
}

void Kernel::run(ModuleId id) {
    jsassert(isModule(id));
    modules[id]->process();
}

void Kernel::step(ModuleId id) {
    jsassert(state == State::READY || state == State::RUNNING_SEQ);

    state = State::RUNNING_SEQ;

    // FIXME Module::setup() is not called when running sequentially

    fetch(id);
    run(id);
    dump(id);
}

void Kernel::setup() {
    jsassert(state == State::READY);
    for(auto loader : loaders) {
        loader->setup();
    }
    
    for (ModuleId m = 0; m < modules.size(); m++) {
        if (isModule(m)) {
            modules[m]->setup();
        }
    }

    isSetup = true;
}

void Kernel::start() {
    jsassert(state == State::READY);

    mutexes.resize(modules.size());
    onReady.resize(modules.size());

    if(!isSetup) {
        setup();
    }

    state = State::RUNNING_ASYNC;

    for (ModuleId m = 0; m < modules.size(); m++) {
        if (isModule(m)) {
            CreateModuleThread(this, m, -1);
        }
    }
}

void Kernel::stop() {
    initiateShutdown();
    state = State::FINISHED;
    LOG_DEBUG << "kernel stopped";
}

void Kernel::moduleLoop(ThreadContext* context, ModuleId moduleId, int runs = -1) {
    jsassert(isModule(moduleId));
    context->notifyReady();
    numRunning.fetch_add(1);

    for (int it = 0; it < runs || runs < 0; it++) {
        fetch(moduleId);
        if (state == State::SHUTDOWN) {
            break;
        }
        run(moduleId);
        dump(moduleId);
    }

    modules[moduleId]->stop();
    numRunning.fetch_sub(1);
    moduleStopped.notify_one();
}

void Kernel::fetch(ModuleId id) {
    jsassert(state != State::ERROR && state != State::READY && state != State::SETUP)
            << meta.modules[id].name << " tried to fetch too early...";
    if (state == State::RUNNING_ASYNC || state == State::SHUTDOWN) {
        std::unique_lock lk{mutexes[id]};
        onReady[id].wait(lk, [&]() { return meta.modules[id].ready() || state == State::SHUTDOWN; });
    }
    if (state != State::SHUTDOWN) {
        jsassert(meta.modules[id].ready()) << "Module '" << meta.modules[id].name << "' not ready yet...\n"
                                           << "Module graph looks like this:\n"
                                           << printModules();
        meta.modules[id].doPreProcess();
    }
}

void Kernel::dump(ModuleId id) {
    jsassert(state != State::ERROR && state != State::SETUP);
    meta.modules[id].doPostProcess();
    if (state != State::RUNNING_ASYNC && state != State::SHUTDOWN) {
        return;
    }
    for (ModuleId &other : meta.modules[id].requiredBy) {
        if (meta.modules[other].ready()) {
            onReady[other].notify_one();
        }
    }
}

void Kernel::initiateShutdown() {
    jsassert(state == State::RUNNING_ASYNC);
    state = State::SHUTDOWN;
    for (ModuleId m = 0; m < modules.size(); m++) {
        onReady[m].notify_one();
    }

    {
        std::unique_lock lock(mtx);
        moduleStopped.wait_for(lock, 2s, [&]() {
            return numRunning.load() == 0;
        });
    }

    for(auto loader : loaders) {
        loader->stop();
    }
}

std::vector<std::shared_ptr<BlackboardBase>> Kernel::blackboards() {
    std::vector<std::shared_ptr<BlackboardBase>> bbs{};
    for (ModuleId m = 0; m < modules.size(); m++) {
        bbs.insert(bbs.end(), meta.modules[m].blackboards.begin(), meta.modules[m].blackboards.end());
    }
    return bbs;
}

std::string Kernel::printModules() const {
    std::stringstream ss{};

    for (auto &module : meta.modules) {
        if (isModule(module.id)) {
            ss << "MODULE";
        } else {
            ss << "HOOK";
        }

        ss << " " << module.name << " [id = " << module.id << "]" << std::endl;

        for (EndpointId endpointId : module.endpoints) {
            const EndpointMeta &endpoint = meta.endpoints.at(endpointId);
            const ChannelMeta &chan = meta.channels.at(endpoint.channel);

            ss << "  ";

            switch (chan.kind) {
                case ChannelMeta::Type::COMMAND:
                    if (endpoint.kind == EndpointMeta::Direction::IN) {
                        ss << "HANDLES";
                    } else {
                        ss << "ISSUES";
                    }
                    ss << " " << prettyTypeName(chan.dataType);

                    if (endpoint.kind == EndpointMeta::Direction::OUT) {
                        ss << " -> " << meta.modules.at(meta.endpoints.at(meta.firstIn(chan.id)).module).name;
                    }
                    break;

                case ChannelMeta::Type::CONTEXT:
                    if (endpoint.kind == EndpointMeta::Direction::IN) {
                        ss << "READS CONTEXT";
                    } else {
                        ss << "WRITES CONTEXT";
                    }
                    ss << " " << prettyTypeName(chan.dataType);

                    if (endpoint.kind == EndpointMeta::Direction::IN) {
                        ss << " <- " << meta.modules.at(meta.endpoints.at(meta.firstOut(chan.id)).module).name;
                    }
                    break;

                case ChannelMeta::Type::BLOB:
                case ChannelMeta::Type::MESSAGE:
                    if (endpoint.kind == EndpointMeta::Direction::IN) {
                        if (endpoint.required) {
                            ss << "REQUIRES";
                        } else {
                            ss << "LISTENS";
                        }
                    }
                    else {
                        ss << "PROVIDES";
                    }
                    ss << " " << prettyTypeName(chan.dataType);

                    if (endpoint.kind == EndpointMeta::Direction::IN) {
                        ss << " <- " << meta.modules.at(meta.endpoints.at(meta.firstOut(chan.id)).module).name;
                    }
                    break;
            }

            ss << std::endl;
        }

        for (auto bb : module.blackboards) {
            ss << "  "
               << "BLACKBOARD " << bb->getBlackboardName() << std::endl;
        }

        for (auto r : module.requiredBy) {
            ss << "  "
               << "REQUIRED BY " << this->meta.modules[r].name << std::endl;
        }
    }

    return ss.str();
}
