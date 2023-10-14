#include "simplethreadmanager.h"
#include <spdlog/spdlog.h>
#include <frontend/framework/thread/util.h>

using namespace spdlog;

enum class NaoThread {
    NORMAL = 0,
    MOTION = 1,
    IO = 2
};

template<>
struct ThreadProperties<NaoThread> {
    static void onCreate(NaoThread type, std::thread &thread) {
        switch(type) {
            case NaoThread::NORMAL:
                info("Create Normal Thread");
                break;
            case NaoThread::MOTION:
                info("Create Motion Thread");
                break;
            case NaoThread::IO:
                info("Create IO Thread");
                break;
        }
    }
};

ThreadManager* GetThreadManager() {
    using T = SimpleThreadManager<NaoThread>;
    static ThreadManager manager(new T());
    return &manager;
}

int main() {
    auto manager = GetThreadManager();
    auto pool = manager->create_pool(NaoThread::MOTION);
    pool->start();
    for (int i = 0; i < 10; i++) {
        pool->submit([i](){
            info("Hello World! {}", i);
        });
    }
    sleep_for(1s);
    pool->stop();
}
