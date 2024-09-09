#pragma once
#include "logdata_impl.h"
#include "../meta.h"

#include <framework/util/assert.h>

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>

namespace rt {

class Linker;
class Kernel;
struct Metadata;

struct LogDataMeta {
    std::mutex mtx;
    std::condition_variable onPush;
};

class LogDataStorage {
public:
    using data_type = LogData;
    using capacity = boost::lockfree::capacity<1000>;
    using queue_type = boost::lockfree::spsc_queue<data_type, capacity>;
    
    ModuleId id{0};

    LogDataStorage(LogDataMeta *meta)
        : meta(meta) {

    }

    void write(const data_type &data) {
        queue.push(data);
        meta->onPush.notify_all();
    }

    template<typename Functor>
    void consume_all(Functor &f) {
        consume_all_internal(f);
    }

    template<typename Functor>
    void consume_all(const Functor &f) {
        consume_all_internal(f);
    }

    bool empty() {
        return queue.empty();
    }

private:
    queue_type queue;
    LogDataMeta *meta;

    template<typename Functor>
    void consume_all_internal(const Functor &f) {
        queue.consume_all(f);
    }
};

struct LogDataContext {
    using logger_type = std::shared_ptr<rt::LogDataStorage>;
    using logger_storage_type = std::vector<logger_type>;
    using iterator = logger_storage_type::iterator;
    using const_iterator = logger_storage_type::const_iterator;

    rt::LogDataStorage& at(size_t id) {
        jsassert(id < loggers.size());
        auto &logger = loggers[id];
        return *logger;
    }

    size_t addLogger(rt::ModuleMeta *module) {
        auto logger = std::make_shared<rt::LogDataStorage>(&logMeta);
        loggers.push_back(logger);
        return loggers.size() - 1; 
    }

    template<typename Rep, typename Period>
    void wait(const std::chrono::duration<Rep, Period>& duration) {
        std::unique_lock lock(logMeta.mtx);
        logMeta.onPush.wait_for(lock, duration, [&]() {
            return hasData();
        });
    }

    rt::ModuleMeta getModule(const logger_type &logger) const {
        return meta->modules[logger->id];
    }

    iterator begin() {
        return loggers.begin();
    }
    
    iterator end() {
        return loggers.end();
    }
    
    const_iterator begin() const {
        return loggers.begin();
    }

    const_iterator end() const {
        return loggers.end();
    }

    bool hasData() {
        bool hasData = false;
        for(auto &logger : loggers) {
            hasData |= not logger->empty();
        }
        return hasData;
    }

private:
    friend Linker;
    friend Kernel;

    const rt::Metadata* meta{nullptr};
    logger_storage_type loggers;
    LogDataMeta logMeta;
};

} // namespace rt
