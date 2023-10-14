#pragma once

#include "channels.h"
#include "meta.h"
#include "module.h"
#include "util/static_vector.h"
#include "util/util.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>
#include <functional>

class ThreadContext;
class BlackboardBase;

namespace rt {

class Kernel;
extern void CreateModuleThread(Kernel* kernel, ModuleId m, int runs);

class Kernel {
public:
    using ConnectFn = std::function<void(Linker&)>;
    using SetupFn = std::function<void()>;
    using CompileResult = std::pair<bool, std::string>;

    Kernel() = default;

    RT_DISABLE_COPY(Kernel)

    ModuleId load(ModuleBase *module);
    ModuleId loadLogger(ModuleBase *module);

    ModuleId hook(std::string_view name, ConnectFn connect, SetupFn setup = {});

    CompileResult compile();
    void setup();
    void start();
    void stop();

    bool isRunning() const;

    std::string printModules() const;

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

    struct LinkContext {
        ModuleMeta moduleMeta;
        ConnectFn connect;
        SetupFn setup;

        LinkContext(ModuleMeta moduleMeta, ConnectFn connect, SetupFn setup)
            : moduleMeta(moduleMeta), connect(connect), setup(setup) {}
    };

    std::mutex mtx;
    std::atomic<int> numRunning{0};
    std::condition_variable moduleStopped;

    bool isSetup = false;
    ContextPool context;
    Metadata meta;

    bool loggerAttached = false;

    std::vector<ModuleBase*> modules;
    std::vector<LinkContext> linkContexts;

    StaticVector<std::mutex> mutexes;
    StaticVector<std::condition_variable> onReady;

    State state = State::SETUP;

    bool isModule(ModuleId) const;
    bool isReady(ModuleId) const;

    ModuleId load(ModuleBase *, ModuleMeta &moduleMeta);
    ModuleId addModule(ModuleBase *, LinkContext &&context);

    void moduleLoop(ThreadContext*, ModuleId, int);
    void resolveDependencies();

    void link();
    CompileResult resolve();

    // Fetch new data from all dependencies of a module.
    // Will block until all new data is available for required entries.
    void fetch(ModuleId);
    void dump(ModuleId);
    void tryRun(ModuleId);
    void run(ModuleId);
    
    void step(ModuleId);
};

} // namespace rt
