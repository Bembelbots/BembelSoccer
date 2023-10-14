#pragma once

#include "channels.h"
#include "meta.h"
#include "module.h"
#include "static_vector.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

class ThreadContext;
class BlackboardBase;

namespace rt {

class Kernel;
extern void CreateModuleThread(Kernel* kernel, ModuleId m, int runs);

class Kernel {

public:
    Kernel() = default;

    Kernel(const Kernel &) = delete;
    Kernel(Kernel &&) = delete;
    Kernel &operator=(const Kernel &) = delete;
    Kernel &operator=(Kernel &&) = delete;

    ModuleId load(Module *module) {
        jsassert(state == State::SETUP);
        Linker l{context, meta};
        module->connect(l);
        ModuleId id = addModule(module, l.finish());
        module->load(*this);
        return id;
    }
    
    ModuleId load(ModuleLoader *loader) {
        jsassert(state == State::SETUP);
        Linker l{context, meta};
        l.name = "loader";
        loader->connect(l);
        ModuleId id = addModule(nullptr, l.finish());
        loader->load(*this);
        loaders.emplace_back(loader);
        return id;
    }

    template<class F>
    ModuleId hook(std::string_view name, F &&connect) {
        jsassert(state == State::SETUP);
        Linker l{context, meta};
        connect(l);
        l.name = name;
        ModuleId id = l.finish();
        return addModule(nullptr, id);
    }

    inline bool isReady(ModuleId id) { return meta.modules.at(id).ready(); }
    void step(ModuleId);

    std::pair<bool, std::string> resolve();

    void setup();
    void start();
    void stop();

    // Fetch new data from all dependencies of a module.
    // Will block until all new data is available for required entries.
    void fetch(ModuleId);
    void dump(ModuleId);

    inline bool isRunning() const { return state == State::RUNNING_SEQ || state == State::RUNNING_ASYNC; }

    std::string printModules() const;

    std::vector<std::shared_ptr<BlackboardBase>> blackboards();

private:
    friend void CreateModuleThread(Kernel* kernel, ModuleId m, int runs);
    enum class State {
        SETUP,
        READY,
        RUNNING_SEQ,
        RUNNING_ASYNC,
        SHUTDOWN,
        FINISHED,
        ERROR,
    };

    std::mutex mtx;
    std::atomic<int> numRunning{0};
    std::condition_variable moduleStopped;

    bool isSetup = false;
    ContextPool context;
    Metadata meta;

    std::vector<Module*> modules;
    std::vector<ModuleLoader*> loaders;

    StaticVector<std::mutex> mutexes;
    StaticVector<std::condition_variable> onReady;

    State state = State::SETUP;

    static std::string composeError(std::string_view, std::string_view);
    std::string channelError(ChannelId, std::string_view) const;
    std::string moduleError(ModuleId, std::string_view) const;

    void resolveDependencies();
    void tryRun(ModuleId);
    void run(ModuleId);

    void moduleLoop(ThreadContext*, ModuleId, int);
    void initiateShutdown();

    ModuleId addModule(Module *, ModuleId);

    inline bool isModule(ModuleId id) const { return modules[id] != nullptr; }
};

} // namespace rt
