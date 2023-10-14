#pragma once
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <functional>
#include "threadmanager.h"
#include "../logger/logger.h"

template<typename PriorityT>
class SimpleThreadPool : public ThreadPool {
public:
    SimpleThreadPool(ThreadManager* manager, PriorityT priority) 
        : manager(manager), priority(priority) {

    }

    void start(uint32_t num_threads = std::thread::hardware_concurrency()) override {
        for (uint32_t i = 0; i < num_threads; i++) {
            manager->create(priority, std::bind(&SimpleThreadPool::worker, this));
        }
    }
    
    void stop() override {
        {
            std::unique_lock lock(mtx);
            running = false;
        }
        onNewJob.notify_all();
    }

    void submit(job_t fn) override {        
        {
            std::unique_lock lock(mtx);
            queue.push(fn);
        }
        onNewJob.notify_one();
    }

private:
    ThreadManager *manager = nullptr;
    PriorityT priority;

    std::queue<job_t> queue;
    std::mutex mtx;
    std::condition_variable onNewJob;
    bool running = true;

    bool isRunning() const {
        return running && manager->isRunning();
    }

    void worker() {
        using namespace std::chrono;
        while(isRunning()) {
            job_t job;
            {
                std::unique_lock lock(mtx);
                auto timeout = high_resolution_clock::now() + milliseconds(1);
                bool res = onNewJob.wait_until(lock, timeout, [this](){ return !queue.empty() || !isRunning(); });
                if(queue.empty() || !isRunning()) {
                    continue;
                }
                job = queue.front();
                queue.pop();
            }
            job();
        }
    }
};

template<typename ThreadT = Thread, typename PoolT = SimpleThreadPool<ThreadT>>
struct SimpleThreadManager : public ThreadManagerBase {
    void on_create(uint32_t priority, std::thread &thread) override { 
        ThreadProperties<ThreadT>::onCreate((ThreadT)priority, thread);
    }
    
    std::shared_ptr<ThreadPool> create_pool(uint32_t priority) override {
        return std::make_shared<PoolT>(manager, (ThreadT)priority);
    }

    std::type_index thread_type() override {
        return typeid(ThreadT);
    }
};
