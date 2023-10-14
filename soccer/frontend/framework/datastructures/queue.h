#pragma once
#include <queue>
#include <mutex>
#include <atomic>

template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue()
        : count(0), storage() {

    }

    void push(const T &item) {
        std::scoped_lock lock(mtx);
        storage.push(item);
        count.fetch_add(1);
    }
    
    void push(T &&item) {
        std::scoped_lock lock(mtx);
        storage.push(item);
        count.fetch_add(1);
    }
   
    template<typename ... Args>
    decltype(auto) emplace(Args&& ... args) {
        std::scoped_lock lock(mtx);
        auto item = storage.emplace(std::forward<Args>(args)...);
        count.fetch_add(1);
        return item;
    }

    T& pop() {
        std::scoped_lock lock(mtx);
        auto &item = storage.front();
        storage.pop();
        count.fetch_sub(1);
        return item;
    }

    void clear() {
        std::scoped_lock lock(mtx);
        std::queue<T> tmp;
        storage = tmp;
        count.exchange(0);
    }

    size_t size() const {
        return count.load();
    }
    
    size_t empty() const {
        return count.load() == 0;
    }

private:
    std::atomic<size_t> count {0};
    std::queue<T> storage;
    std::mutex mtx;
};
