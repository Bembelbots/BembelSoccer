#pragma once
#include "endpoints.h"
#include "../thread/task.h"
#include "../thread/threadmanager.h"
#include <tuple>
#include <functional>
#include <memory>
#include <thread>
#include <iostream>

namespace rt {

    template<typename T, typename StateT>
    struct TaskPool {
        using channel_t = TaskChannel<T, StateT>;
        std::shared_ptr<channel_t> channel;
        
        TaskPool() : channel(std::make_shared<channel_t>()) {
            manager = GetThreadManager();
        }

        ThreadManager *manager = nullptr;
        std::shared_ptr<ThreadPool> pool; 
    };

    template<typename T, typename StateT>
    class TaskOutput : public Output<T> {
    public:
        using taskpool_t = TaskPool<T, StateT>;
        using channel_t = typename taskpool_t::channel_t;

        template<typename ThreadType>
        void create_pool(ThreadType type, int num_threads = std::thread::hardware_concurrency()) {
            jsassert(!link->pool);
            auto &thelink = assertLink();
            thelink.pool = thelink.manager->create_pool(type);
            thelink.pool->start(num_threads);
        }

        void submit(T &data) {
            assertLink().channel->submit(StateT::COMPLETE, data);
        }
        
        void submit(T &&data) {
            assertLink().channel->submit(StateT::COMPLETE, data);
        }

        typename channel_t::complete_queue_type fetchComplete() {
            auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1);
            return assertLink().channel->takeComplete(timeout);
        }

        ~TaskOutput() {
            assertLink().channel->close();
        }

    private:
        friend class Linker;
        taskpool_t *link = nullptr;
        
        auto &assertLink() {
            jsassert(link != nullptr);
            return *link;
        }
    };

    template<typename T, typename StateT, StateT ListenState>
    class TaskInput {
    public:
        using taskpool_t = TaskPool<T, StateT>;
        using channel_t = typename taskpool_t::channel_t;

        typename channel_t::queue_type fetch() {
            auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1);
            return assertLink().channel->take(id, timeout);
        }

        template<typename Fn>
        void asyncFetch(Fn fn) {
            for(auto &ticket : fetch()) {
                async([ticket, fn]() {
                    fn(ticket->get_task());
                    ticket->release();
                });
            }
        } 

    private:
        friend class Linker;

        int id = -1;
        taskpool_t *link = nullptr;
        
        void async(typename ThreadPool::job_t job) {
            auto &link = assertLink();
            jsassert(link.pool);
            link.pool->submit(job);
        }

        auto &assertLink() {
            jsassert(link != nullptr && id != -1);
            return *link;
        }
    };


}
