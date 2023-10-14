#pragma once

#include "../message_utils.h"

#include <mutex>
#include <vector>
#include <condition_variable>
#include <optional>
#include <functional>

namespace rt {

template<typename T>
class MessageChannel {
public:
    using TapFunction = std::function<void(const T &)>;

    int addListener() {
        listeners.emplace_back(new ListenerCtx{});
        return listeners.size() - 1;
    }

    int addSnoopingListener() {
        snoopers.emplace_back(new SnoopingCtx{});
        return snoopers.size() - 1;
    }
    
    void addTap(TapFunction &&fn) {
        taps.emplace_back(std::forward<TapFunction>(fn));
    }

    void write(const T &data) {
        for (auto func : taps) {
           func(data); 
        }

        for (auto &ctx : listeners) {
            std::lock_guard lock{ctx->mtx};
            if (ctx->queue == std::nullopt) {
                ctx->queue = data;
            } else {
                squash<T>(*ctx->queue, data);
            }
        }

        for (auto &ctx : snoopers) {
            std::lock_guard lock{ctx->mtx};
            ctx->queue.push_back(data);
            ctx->onNewEntry.notify_one();
        }
    }

    bool hasNewData(int id) const { return assureListener(id).queue.has_value(); }

    void fetch(int id, T &out) {
        auto &ctx = assureListener(id);
        std::lock_guard lock{ctx.mtx};
        jsassert(ctx.queue.has_value());
        out = *ctx.queue;
        ctx.queue = std::nullopt;
    }

    void maybeFetch(int id, T &out) {
        auto &ctx = assureListener(id);
        std::lock_guard lock{ctx.mtx};
        if (ctx.queue.has_value()) {
            out = *ctx.queue;
            ctx.queue = std::nullopt;
        }
    }

    void waitWhileEmpty(int id) {
        using namespace std::chrono;
        auto &ctx = assureSnoop(id);
        std::unique_lock lock{ctx.mtx};
        auto timeout = high_resolution_clock::now() + milliseconds(1);
        ctx.onNewEntry.wait_until(lock, timeout, [&]() { return not ctx.queue.empty(); });
    }

    std::vector<T> snoopFetch(int id) {
        auto &ctx = assureSnoop(id);
        std::lock_guard lock{ctx.mtx};
        std::vector<T> out = std::move(ctx.queue);
        ctx.queue.clear();
        return out;
    }

private:
    struct ListenerCtx {
        std::mutex mtx;
        std::optional<T> queue;
    };

    struct SnoopingCtx {
        std::mutex mtx;
        std::condition_variable onNewEntry;
        std::vector<T> queue;
    };

    std::vector<std::unique_ptr<ListenerCtx>> listeners;
    std::vector<std::unique_ptr<SnoopingCtx>> snoopers;

    std::vector<TapFunction> taps;

    ListenerCtx &assureListener(int id) {
        jsassert(id > -1 && size_t(id) < listeners.size());
        return *listeners[id];
    }

    const ListenerCtx &assureListener(int id) const {
        jsassert(id > -1 && size_t(id) < listeners.size());
        return *listeners[id];
    }

    SnoopingCtx &assureSnoop(int id) {
        jsassert(id > -1 && size_t(id) < snoopers.size());
        return *snoopers[id];
    }

    const SnoopingCtx &assureSnoop(int id) const {
        jsassert(id > -1 && size_t(id) < snoopers.size());
        return *snoopers[id];
    }
};

} // namespace rt
