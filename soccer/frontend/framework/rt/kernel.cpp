#include "kernel.h"

#include "util/depth_first_search.h"
#include "meta.h"

#include "../blackboard/blackboard.h"
#include "../util/assert.h"
#include "../thread/util.h"
#include "../thread/threadmanager.h"

#include <iostream>
#include <sstream>

using namespace rt;

bool Kernel::isModule(ModuleId id) const {
    return modules[id] != nullptr;
}
    
bool Kernel::isReady(ModuleId id) const {
    return meta.modules.at(id).ready();
}

bool Kernel::isRunning() const {
    return state == State::RUNNING_SEQ || state == State::RUNNING_ASYNC;
}

ModuleId Kernel::addModule(ModuleBase* module, LinkContext &&link) {
    jsassert(state == State::SETUP);
    modules.emplace_back(module);
    linkContexts.emplace_back(link);
    return modules.size();
}

ModuleId Kernel::load(ModuleBase* module, ModuleMeta &moduleMeta) {
    using namespace std::placeholders;
    jsassert(module != nullptr);
    jsassert(state == State::SETUP);
    if(module->disabled()) {
        return INVALID_ID;
    }
    moduleMeta.tags |= module->type();
    auto connect    = std::bind(&Module::connect, module, _1);
    auto setup      = std::bind(&Module::setup, module);
    LinkContext context (moduleMeta, std::move(connect), std::move(setup));
    ModuleId id = addModule(module, std::move(context));
    module->load(*this);
    return id;
}

ModuleId Kernel::load(ModuleBase *module) {
    jsassert(state == State::SETUP);
    if(module->disabled()) {
        return INVALID_ID;
    }
    ModuleMeta moduleMeta;
    return load(module, moduleMeta);
}

ModuleId Kernel::loadLogger(ModuleBase *module) {
    jsassert(state == State::SETUP);
    jsassert(loggerAttached == false); // only one logger allowed
    if(module->disabled()) {
        return INVALID_ID;
    }
    ModuleMeta moduleMeta;
    moduleMeta.tags |= ModuleTag::Logger;
    loggerAttached = true;
    return load(module, moduleMeta);
}

ModuleId Kernel::hook(std::string_view name, ConnectFn connect, SetupFn setup) {
    jsassert(state == State::SETUP);
    ModuleMeta moduleMeta;
    moduleMeta.name = name;
    moduleMeta.tags |= ModuleTag::Hook | ModuleTag::NoThread;
    return addModule(nullptr, LinkContext(moduleMeta, connect, setup));
}

Kernel::CompileResult Kernel::compile() {
    link();
    return resolve();
}

void Kernel::link() {
    ModuleTag extra = ModuleTag::None;

    if(not loggerAttached) {
        LOG_WARN << "no logger attached disabling logging!";
        extra |= ModuleTag::DisableLogging;
    }

    for (ModuleId m = 0; m < modules.size(); m++) {
        LinkContext &module = linkContexts.at(m);
        Linker l(context, meta);
        l.module = module.moduleMeta;
        l.name = l.module.name;
        l.module.tags |= extra;
        module.connect(l);
        jsassert(m == l.finish());
    }
}

Kernel::CompileResult Kernel::resolve() {
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
        errorAcc << meta.moduleError(
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
                    errorAcc << meta.channelError(chan.id, "Message has no producer.");
                    nErrors++;
                }
                if (producers.size() >= 2) {
                    errorAcc << meta.channelError(chan.id, "Message has more than one producer.");
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
                    errorAcc << meta.channelError(chan.id, "Commands have no handler.");
                    nErrors++;
                }
                if (handlers.size() >= 2) {
                    errorAcc << meta.channelError(chan.id, "Commands have multiple handlers.");
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
    jsassert(not tag_set(meta.modules[id].tags, ModuleTag::NoThread));
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
    
    auto &logdata = context.get<LogDataContext>();
    logdata.meta = &meta;

    for (ModuleId m = 0; m < modules.size(); m++) {
        LinkContext &module = linkContexts.at(m);
        if (module.setup) {
            module.setup();
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
        if(not isModule(m)) {
            continue;
        }
        if (not tag_set(meta.modules[m].tags, ModuleTag::NoThread)) {
            CreateModuleThread(this, m, -1);
        }
    }
}

void Kernel::stop() {
    jsassert(state == State::RUNNING_ASYNC);

    state = State::SHUTDOWN;

    for (ModuleId m = 0; m < modules.size(); m++) {
        onReady[m].notify_one();
    }

    std::unique_lock lock(mtx);
    moduleStopped.wait_for(lock, 2s, [&]() {
        return numRunning.load() == 0;
    });
    
    for (ModuleId m = 0; m < modules.size(); m++) {
        if(not isModule(m)) {
            continue;
        }
        if (tag_set(meta.modules[m].tags, ModuleTag::NoThread)) {
            modules[m]->stop();
        }
    }

    state = State::FINISHED;

    LOG_DEBUG << "kernel stopped";
}

void Kernel::moduleLoop(ThreadContext* context, ModuleId moduleId, int runs = -1) {
    jsassert(isModule(moduleId));
    jsassert(not tag_set(meta.modules[moduleId].tags, ModuleTag::NoThread));
    context->notifyReady();
    numRunning.fetch_add(1);

    set_current_thread_name(meta.modules[moduleId].name.c_str());

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

std::string Kernel::printModules() const {
    std::stringstream ss{};

    for (auto &module : meta.modules) {
        if(tag_set(module.tags, ModuleTag::Hook)) {
            ss << "HOOK";
        } else if(tag_set(module.tags, ModuleTag::Logger)) {
            ss << "LOGGER";
        } else if(tag_set(module.tags, ModuleTag::Normal)) {
            ss << "MODULE";
        } else if(tag_set(module.tags, ModuleTag::NoThread)) {
            ss << "NT MODULE";
        }

        ss << " " << meta.printModule(module);
    }

    return ss.str();
}
