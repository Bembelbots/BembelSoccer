#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <future>
#include <atomic>
#include <chrono>
#include <iostream>
#include <exception>
#include "../util/assert.h"
#include "../logger/logger.h"

class InvalidListenerId : public std::exception {
private:
    int id;

public:
    InvalidListenerId(int _id)
        : std::exception(), id(_id) {

    }

    const char * what() const throw() override {
        return "Invalid listener id";
    }
};

template<typename Task>
struct TaskContext {
    std::shared_ptr<Task> task;
    std::atomic<int> count; 
};

template<typename Task>
struct TaskTicket {
    explicit TaskTicket(std::shared_ptr<Task> task)
        : task(task) {
    }

    ~TaskTicket() {
        release();
    }

    void release() {
        if(released) { return; }
        released = true;
        task->channel->setDone(task);
    }

    Task* get_task() {
        return task.get();
    }

private:
    bool released = false;
    std::shared_ptr<Task> task;
};

template<typename DataT, typename StateT>
struct Task;

template<typename DataT, typename StateT>
struct TaskChannel {
    using task_type = Task<DataT, StateT>;
    using state_type = StateT;
    using queue_type = std::vector<std::shared_ptr<TaskTicket<task_type>>>;
    using complete_queue_type = std::vector<std::shared_ptr<task_type>>;

    template<typename ...Args>
    void submit(Args && ...args) {
        if(closed) { return; } 
        auto task = createTask(std::forward<Args>(args)...);
        submit_existing(task);
    }

    int listen(const state_type &state) {
        if(closed) { return -1; }
        std::scoped_lock lock(mtx);
        auto ctx = std::make_shared<ListenerContext>();
        ctx->state = state;
        listeners.push_back(ctx);
        return listeners.size() - 1;
    }

    queue_type take(int id) {
        if(closed) { return {}; }
        auto listener = assure(id);
        std::unique_lock lock(listener->mtx);
        if(!wait(lock, listener)) { return {}; }
        auto current = std::move(listener->queue);
        listener->queue.clear();
        return current;
    }
   
    template<typename Clock, typename Duration>
    queue_type take(int id, std::chrono::time_point<Clock, Duration> timeout) {
        if(closed) { return {}; }
        auto listener = assure(id);
        std::unique_lock lock(listener->mtx);
        if(!wait(lock, listener, timeout)) { return {}; }
        auto current = std::move(listener->queue);
        listener->queue.clear();
        return current;
    }

    template<typename Clock, typename Duration>
    complete_queue_type takeComplete(std::chrono::time_point<Clock, Duration> timeout) {
        if(closed) { return {}; }
        std::unique_lock lock(mtx);
        if(!waitComplete(lock, timeout)) { return {}; }
        auto current = complete;
        complete.clear();
        return current;
    }

    complete_queue_type takeComplete() {
        if(closed) { return {}; }
        std::unique_lock lock(mtx);
        if(!waitComplete(lock)) { return {}; }
        auto current = complete;
        complete.clear();
        return current;
    }

    void close() {
        if(closed) { return; }
        {
            std::scoped_lock lock(mtx);
            closed = true;
        }
        onNewTask.notify_all();
        onComplete.notify_all();
    }

private:
    friend task_type;
    friend TaskTicket<task_type>;

    struct ListenerContext {
        state_type state;
        queue_type queue;
        std::mutex mtx;
    };

    std::vector<std::shared_ptr<ListenerContext>> listeners;
    
    complete_queue_type complete;
    std::atomic<bool> closed = false;

    std::mutex mtx;
    std::condition_variable onNewTask;
    std::condition_variable onComplete;

    std::shared_ptr<ListenerContext> assure(int id) {
        if(id < 0 || id >= listeners.size()) {
            throw InvalidListenerId(id);
        }
        return listeners[id];
    }

    bool wait(std::unique_lock<std::mutex>& lock, std::shared_ptr<ListenerContext> listener) {
        onNewTask.wait(lock, [&](){ return !listener->queue.empty() || closed; });
        return !listener->queue.empty();
    }
    
    template<typename Clock, typename Duration>
    bool wait(std::unique_lock<std::mutex>& lock, std::shared_ptr<ListenerContext> listener, std::chrono::time_point<Clock, Duration> timeout) {
        onNewTask.wait_until(lock, timeout, [&](){ return !listener->queue.empty() || closed; });
        return !listener->queue.empty();
    }

    template<typename Clock, typename Duration>
    bool waitComplete(std::unique_lock<std::mutex>& lock, std::chrono::time_point<Clock, Duration> timeout) {
        onComplete.wait_until(lock, timeout, [&](){ return !complete.empty() || closed; });
        return !complete.empty();
    }

    bool waitComplete(std::unique_lock<std::mutex>& lock) {
        onComplete.wait(lock, [&](){ return !complete.empty() || closed; });
        return !complete.empty();
    }
    
    template <typename ...Args>
    std::shared_ptr<task_type> createTask(Args && ...args) {
        auto task = std::make_shared<task_type>(std::forward<Args>(args)...);
        auto tc = std::make_shared<TaskContext<task_type>>(); 
        tc->task = task;
        task->setContext(std::move(tc)); 
        task->setChannel(this);
        return task;
    }

    bool trySubmitTask(std::shared_ptr<task_type> &task) {
        if(closed) { return false; }
        std::scoped_lock lock(mtx);

        jsassert(task);
        if(task->complete()) {
            task->context->task = nullptr;
            complete.push_back(std::move(task));
            onComplete.notify_all();
            return true;
        }
 
        int count = 0;
        for(auto &listener : listeners) {
            if(listener->state != task->current_state) {
                continue;
            }
            count++;
        }

        if(count == 0) {
            return false;
        }

        task->context->count.fetch_add(count);

        for(auto &listener : listeners) {
            if(listener->state != task->current_state) {
                continue;
            }
            {
                std::scoped_lock lock(listener->mtx);
                listener->queue.emplace_back(std::make_shared<TaskTicket<task_type>>(task));
            }
            onNewTask.notify_one();
        }
        onNewTask.notify_all();
        return true;
    }

    void submit_existing(std::shared_ptr<task_type> &task) {
        while(!trySubmitTask(task)) {
            std::scoped_lock lock(mtx);
            if(closed) { return; }
            task->iterate();
        }
    }

    void setDone(std::shared_ptr<task_type> &task) {
        if(closed) { return; }
        task->context->count.fetch_sub(1); 
        if(task->context->count > 0) {
            return;
        }
        {
            std::scoped_lock lock(mtx);
            task->iterate();
        }
        submit_existing(task);
    }
};

template<typename DataT, typename StateT>
struct Task {
    using data_type = DataT;
    using state_type = StateT;
    using channel_type = TaskChannel<DataT, StateT>;
    using context_type = TaskContext<Task>;

    data_type data;
    state_type complete_state;
    state_type current_state;

    Task(const state_type &_complete_state, data_type &&_data)
        :   data(std::move(_data)),
            complete_state(_complete_state),
            current_state() {
    }
    
    Task(const state_type &_complete_state, data_type _data)
        :   data(std::move(_data)),
            complete_state(_complete_state),
            current_state() {
    }

    bool complete() const {
        return current_state == complete_state;
    }

private:
    friend channel_type;
    friend TaskTicket<Task>;

    channel_type* channel = nullptr;
    std::shared_ptr<context_type> context = nullptr;

    void setChannel(channel_type* channel) {
        this->channel = channel;
    }

    void setContext(std::shared_ptr<context_type> context) {
        this->context = context;
    }
    
    void iterate() {
        if(complete()) {
            return;
        }
        current_state = state_type(current_state + 1);
    }
};
