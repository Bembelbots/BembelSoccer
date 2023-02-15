#pragma once

#include "message_utils.h"
#include "ring_buffer.h"
#include "type_info.h"

#include "../logger/logger.h"
#include "../util/assert.h"

#include <entt/signal/dispatcher.hpp>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <vector>
#include <chrono>

namespace rt {

template<typename T>
class MessageChannel {

public:
    int addListener() {
        listeners.emplace_back(new ListenerCtx{});
        return listeners.size() - 1;
    }

    int addSnoopingListener() {
        snoopers.emplace_back(new SnoopingCtx{});
        return snoopers.size() - 1;
    }

    void write(const T &data) {
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

template<typename T, typename = typename std::enable_if_t<is_blob_v<T>>>
class BlobChannel {

public:
    using DataT = typename T::DataT;
    using PayloadT = typename T::PayloadT;

    static constexpr bool HasPayload = T::HasPayload;
    static constexpr size_t Alignment = 64;

    BlobChannel() : size(), buffer(Alignment, size.flat()) {
        addEntry();
        addEntry();
    }

    int addListener() {
        readers.emplace_back();
        addEntry();
        return readers.size() - 1;
    }

    void addWriter(const Dim &size) {
        std::lock_guard lock{mtx};

        this->size = size;
        buffer.setSize(size.flat());
        addListener();
    }

    void startWriting(T &blob) {
        std::lock_guard lock{mtx};
        blob.data = buffer.startWriting();
        blob.size = size;
    }

    void finishWriting(T &blob) {
        std::lock_guard lock{mtx};

        setPayload(buffer.getWriteBuf(), blob.info);
        buffer.finishWriting();
        blob.data = nullptr;
        blob.size = Dim{};
        tick++;
    }

    bool hasNewData(int id) const { return assureReader(id).tick < tick; }

    void startReading(int id, T &blob, bool requiresNewData = false) {
        std::lock_guard lock{mtx};

        jsassert(requiresNewData ? hasNewData(id) : true);
        auto &ctx = assureReader(id);
        ctx.bufId = buffer.addReader();
        ctx.tick = tick;
        blob.data = buffer.data(ctx.bufId);
        blob.size = size;
        blob.info = getPayload(ctx.bufId);
    }

    void stopReading(int id, T &blob) {
        std::lock_guard lock{mtx};

        buffer.removeReader(assureReader(id).bufId);
        blob.data = nullptr;
        blob.size = Dim{};
        blob.info = PayloadT{};
    }

private:
    struct ReaderCtx {
        size_t tick = 0;
        size_t bufId = RingBuffer<DataT>::NO_REF;
    };

    std::mutex mtx;
    size_t tick = 0;
    Dim size;
    RingBuffer<DataT> buffer;
    std::vector<PayloadT> payload;

    std::vector<ReaderCtx> readers;

    PayloadT getPayload(size_t id) const {
        if constexpr (HasPayload) {
            return payload.at(id);
        } else {
            return NoPayload{};
        }
    }

    void setPayload(size_t id, const PayloadT &p) {
        if constexpr (HasPayload) {
            payload.at(id) = p;
        }
    }

    void addEntry() {
        if constexpr (HasPayload) {
            payload.emplace_back();
        }
        buffer.addEntry();
    }

    ReaderCtx &assureReader(int id) {
        jsassert(id > -1 && size_t(id) < readers.size());
        return readers[id];
    }

    const ReaderCtx &assureReader(int id) const {
        jsassert(id > -1 && size_t(id) < readers.size());
        return readers[id];
    }
};

template<typename CommandGroup>
class CommandChannel {

public:
    template<typename Command, auto Candidate,
            typename = typename std::enable_if_t<is_command_v<Command, CommandGroup>>>
    void connect() {
        std::lock_guard lock{mtx};
        dispatcher.sink<Command>().template connect<Candidate>();
    }

    template<typename Command, auto Candidate, typename Type,
            typename = typename std::enable_if_t<is_command_v<Command, CommandGroup>>>
    void connect(Type &&valueOrInstance) {
        std::lock_guard lock{mtx};
        dispatcher.sink<Command>().template connect<Candidate>(valueOrInstance);
    }

    void update() {
        std::lock_guard lock{mtx};
        dispatcher.update();
    }

    template<typename Command, typename = typename std::enable_if_t<is_command_v<Command, CommandGroup>>,
            typename... Args>
    void enqueue(Args &&... args) {
        std::lock_guard lock{mtx};
        dispatcher.template enqueue<Command>(std::forward<Args>(args)...);
    }

private:
    std::mutex mtx;
    entt::dispatcher dispatcher;
};

class ContextPool {

public:
    template<typename T>
    T &get() {
        PoolEntry<T> *entry = nullptr;
        for (auto &e : entries) { // cppcheck-suppress useStlAlgorithm
            if (e->type == TypeInfo<T>::id()) {
                entry = static_cast<PoolEntry<T> *>(e.get());
                break;
            }
        }
        if (entry == nullptr) { // cppcheck-suppress nullPointerRedundantCheck
            entry = new PoolEntry<T>{};
            entries.emplace_back(entry);
        }
        return entry->data; // cppcheck-suppress nullPointerRedundantCheck
    }

private:
    struct BasicPoolEntry {
        TypeID type;

        explicit BasicPoolEntry(TypeID t) : type(t) {}
        virtual ~BasicPoolEntry() {}
    };

    template<typename T>
    struct PoolEntry : public BasicPoolEntry {
        T data;

        PoolEntry() : BasicPoolEntry(TypeInfo<T>::id()) {}
    };

    std::vector<std::unique_ptr<BasicPoolEntry>> entries;
};

} // namespace rt
