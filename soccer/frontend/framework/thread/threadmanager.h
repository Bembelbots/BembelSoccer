#pragma once
#include <vector>
#include <thread>
#include <memory>
#include <functional>
#include <typeindex>
#include <exception>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "../logger/logger.h"

enum class Thread {
    NORMAL = 0,
    LOW_PRIORITY,
    HIGH_PRIORITY,
};
 
class InvalidThreadType: public std::exception {
private:

public:
    InvalidThreadType() : std::exception() {

    }

    const char * what() const throw() override {
        return "Invalid thread type";
    }
};

template<typename T>
struct ThreadProperties {
    static void onCreate(T type, std::thread &) {}
};

class ThreadPool {
public:
    using job_t = std::function<void()>;
    virtual void start(uint32_t num_threads = std::thread::hardware_concurrency()) = 0;
    virtual void stop() = 0;
    virtual void submit(job_t job) = 0;
    virtual ~ThreadPool(){}
};

struct ThreadManager;

struct ThreadManagerBase {
    virtual void attach(ThreadManager *manager) {
        this->manager = manager;
    }
    virtual void on_create(uint32_t type, std::thread &thread) = 0;
    virtual void on_create(std::thread &thread) {
        on_create(0, thread);
    };
    virtual std::shared_ptr<ThreadPool> create_pool(uint32_t type) = 0;
    virtual std::type_index thread_type() = 0;
    virtual ~ThreadManagerBase() = default; 

protected:
    ThreadManager *manager;
};

struct ThreadContext {

    void notifyReady() {
        ready.exchange(true);
        onReady.notify_all();
    }

private:
    friend ThreadManager;
    std::mutex mtx;
    std::atomic<bool> ready;
    std::condition_variable onReady;
};

struct ThreadManager {

    template<typename T>
    explicit ThreadManager(T *ptr)
        : impl(std::shared_ptr<ThreadManagerBase>(ptr)) {
        impl->attach(this);
        thread_t = impl->thread_type();
    }
    
    ~ThreadManager() {
        stop();
    }

    template<typename T, typename Callable, typename ...Args>
    size_t create(T type, Callable &&fn, Args && ...args) {
        using namespace std::chrono_literals;
        std::scoped_lock lock(mtx);
        
        assert_thread_type<T>();
        {
            std::scoped_lock readylock(context.mtx);
            context.ready.exchange(false);
        }
        
        std::thread new_thread(std::forward<Callable>(fn), &context, std::forward<Args>(args)...);
        impl->on_create((uint32_t)type, new_thread);
        
        bool is_ready;
        {
            std::unique_lock readylock(context.mtx);
            is_ready = context.onReady.wait_for(readylock, 1s, [&](){
                return context.ready.load();
            });
        }
        
        if(is_ready) {
            threads.push_back(std::move(new_thread));
        } else {
            LOG_WARN << "thread not ready!";
        }

        return threads.size() - 1;
    }
    
    template<typename T>
    std::shared_ptr<ThreadPool> create_pool(T type) {
        assert_thread_type<T>();
        return impl->create_pool((uint32_t)type);
    }

    void stop() {
        running = false;
        for(auto &thread : threads) {
            if(thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();
    }

    void join(size_t thread_id) {
        // TODO: recycle thread id

        if(thread_id >= threads.size()) {
            return;
        }

        auto &thread = threads[thread_id];

        if(thread.joinable()) {
            thread.join();
        }
    }

    bool isRunning() const {
        return running;
    }

private:
    bool running = true;
    std::mutex mtx;
    std::vector<std::thread> threads;
    std::shared_ptr<ThreadManagerBase> impl;
    std::type_index thread_t = typeid(std::nullptr_t);

    ThreadContext context;
    
    template<typename T>
    void assert_thread_type() const {
        if(thread_t != std::type_index(typeid(T))) {
            throw InvalidThreadType();
        }
    }
};

extern ThreadManager* GetThreadManager();
