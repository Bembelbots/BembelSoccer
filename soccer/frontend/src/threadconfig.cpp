#include <framework/thread/simplethreadmanager.h>
#include <framework/logger/logger.h>
#include <framework/network/network.h>
#include <representations/bembelbots/thread.h>
#include <framework/rt/kernel.h>

struct ThreadStats {
    std::atomic<int> normal = 0;
    std::atomic<int> io = 0;
    std::atomic<int> motion = 0;
    std::atomic<int> total = 0;
};

template<>
struct ThreadProperties<NaoThread> {
    static void onCreate(NaoThread type, std::thread &thread) {
        static ThreadStats stats;
        stats.total.fetch_add(1);
        switch(type) {
            case NaoThread::NORMAL:
                stats.normal.fetch_add(1);
                LOG_DEBUG << "Create Normal Thread " << stats.normal.load();
                break;
            case NaoThread::MOTION:
                stats.motion.fetch_add(1);
                LOG_DEBUG << "Create Motion Thread " << stats.motion.load();
                break;
            case NaoThread::IO:
                stats.io.fetch_add(1);
                LOG_DEBUG << "Create IO Thread " << stats.io.load();
                break;
        }
        LOG_DEBUG << "Total threads " << stats.total.load();
    }
};

ThreadManager* GetThreadManager() {
    using T = SimpleThreadManager<NaoThread>;
    static ThreadManager manager(new T());
    return &manager;
}

void rt::CreateModuleThread(rt::Kernel* kernel, rt::ModuleId m, int runs) {
    using namespace std::placeholders;
    static auto *tm = GetThreadManager();
    tm->create(NaoThread::NORMAL, std::bind(&rt::Kernel::moduleLoop, kernel, _1, _2, _3), m, runs);
}

void CreateNetworkThread(NetworkIO *network) {
    using namespace std::placeholders;
    GetThreadManager()->create(NaoThread::IO, std::bind(&NetworkIO::worker, network, _1));
}

void CreateXLoggerThread(XLogger *logger) {
    using namespace std::placeholders;
    GetThreadManager()->create(NaoThread::IO, std::bind(&XLogger::io_worker, logger, _1));
}
