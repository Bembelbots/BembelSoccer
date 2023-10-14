#pragma once

#include <framework/thread/simplethreadmanager.h>
#include <framework/logger/logger.h>

ThreadManager* GetThreadManager() {
    using T = SimpleThreadManager<Thread>;
    static ThreadManager manager(new T());
    return &manager;
}

void CreateXLoggerThread(XLogger *logger) {
    using namespace std::placeholders;
    GetThreadManager()->create(Thread::NORMAL, std::bind(&XLogger::io_worker, logger, _1));
}
