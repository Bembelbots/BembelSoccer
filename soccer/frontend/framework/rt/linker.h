#pragma once

#include "meta.h"
#include "channels.h"
#include "endpoints.h"
#include "task.h"
#include "type_info.h"
#include <type_traits>

namespace rt {

class Linker {

public:
    std::string name;

    Linker(ContextPool &context, Metadata &meta) : context(context), meta(meta) {}

    template<typename T>
    typename std::enable_if_t<not is_blob_v<T>> operator()(Input<T, Listen> &endpoint) {
        auto &link = context.get<MessageChannel<T>>();
        endpoint.id = link.addListener();
        module.preProcess.emplace_back([&]() { link.maybeFetch(endpoint.id, endpoint.data); });
        addEndpoint(EndpointMeta::Direction::IN, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
    }

    template<typename T>
    typename std::enable_if_t<not is_blob_v<T>> operator()(Input<T, Require> &endpoint) {
        auto &link = context.get<MessageChannel<T>>();
        endpoint.id = link.addListener();
        module.readyFuncs.emplace_back([&]() { return link.hasNewData(endpoint.id); });
        module.preProcess.emplace_back([&]() { link.fetch(endpoint.id, endpoint.data); });
        addEndpoint(EndpointMeta::Direction::IN, true, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
    }

    template<typename T>
    typename std::enable_if_t<not is_blob_v<T>> operator()(Input<T, Snoop> &endpoint) {
        auto &link = context.get<MessageChannel<T>>();
        endpoint.id = link.addSnoopingListener();
        endpoint.link = &link;
        addEndpoint(EndpointMeta::Direction::IN, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
    }
    
    template<typename T>
    typename std::enable_if_t<not is_blob_v<T>> operator()(Input<T, Event> &endpoint) {
        auto &link = context.get<MessageChannel<T>>();
        endpoint.id = link.addSnoopingListener();
        endpoint.link = &link;
        addEndpoint(EndpointMeta::Direction::IN, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
    }

    template<typename T>
    typename std::enable_if_t<is_blob_v<T>> operator()(Input<T, Listen> &endpoint) {
        auto &link = context.get<BlobChannel<T>>();
        endpoint.id = link.addListener();
        module.preProcess.emplace_back([&]() { link.startReading(endpoint.id, endpoint.data); });
        module.postProcess.emplace_back([&]() { link.stopReading(endpoint.id, endpoint.data); });
        addEndpoint(EndpointMeta::Direction::IN, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::BLOB);
    }

    template<typename T>
    typename std::enable_if_t<is_blob_v<T>> operator()(Input<T, Require> &endpoint) {
        auto &link = context.get<BlobChannel<T>>();
        endpoint.id = link.addListener();
        module.readyFuncs.emplace_back([&]() { return link.hasNewData(endpoint.id); });
        module.preProcess.emplace_back([&]() { link.startReading(endpoint.id, endpoint.data, true); });
        module.postProcess.emplace_back([&]() { link.stopReading(endpoint.id, endpoint.data); });
        addEndpoint(EndpointMeta::Direction::IN, true, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::BLOB);
    }

    template<typename T>
    typename std::enable_if_t<not is_blob_v<T>> operator()(Output<T, Default> &endpoint) {
        jsassert(not endpoint.connected);
        auto &link = context.get<MessageChannel<T>>();
        module.postProcess.emplace_back([&]() { link.write(endpoint.data); });
        addEndpoint(EndpointMeta::Direction::OUT, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
        endpoint.connected = true;
    }
    
    template<typename T>
    typename std::enable_if_t<not is_blob_v<T>> operator()(Output<T, Event> &endpoint) {
        jsassert(endpoint.link == nullptr);
        auto &link = context.get<MessageChannel<T>>();
        addEndpoint(EndpointMeta::Direction::OUT, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
        endpoint.link = &link;
    }
    
    template<typename T>
    typename std::enable_if_t<not is_blob_v<T>> operator()(Output<T, Batch> &endpoint) {
        jsassert(not endpoint.connected);
        auto &link = context.get<MessageChannel<T>>();
        addEndpoint(EndpointMeta::Direction::OUT, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
        endpoint.connected = true;
        endpoint.link = &link;
    }

    template<typename T>
    typename std::enable_if_t<is_blob_v<T>> operator()(Output<T, Default> &endpoint, Dim size) {
        jsassert(not endpoint.connected);
        auto &link = context.get<BlobChannel<T>>();
        link.addWriter(size);
        module.preProcess.emplace_back([&]() { link.startWriting(endpoint.data); });
        module.postProcess.emplace_back([&]() { link.finishWriting(endpoint.data); });
        addEndpoint(EndpointMeta::Direction::OUT, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::BLOB);
        endpoint.connected = true;
    }

    template<typename T>
    void operator()(Context<T> &endpoint) {
        auto *data = &context.get<T>();
        endpoint.data = data;
        addEndpoint(EndpointMeta::Direction::IN, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::CONTEXT);
    }

    template<typename T>
    void operator()(Context<T, Write> &endpoint) {
        auto *data = &context.get<T>();
        endpoint.data = data;
        addEndpoint(EndpointMeta::Direction::OUT, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::CONTEXT);
    }

    template<typename Group>
    void operator()(Dispatch<Group> &endpoint) {
        auto *link = &context.get<CommandChannel<Group>>();
        endpoint.link = link;
        addEndpoint(EndpointMeta::Direction::OUT, false, &endpoint, TypeInfo<Group>::id(), ChannelMeta::Type::COMMAND);
    }

    template<typename Group>
    void operator()(Dispatch<Group, Handle> &endpoint) {
        auto *link = &context.get<CommandChannel<Group>>();
        endpoint.link = link;
        addEndpoint(EndpointMeta::Direction::IN, false, &endpoint, TypeInfo<Group>::id(), ChannelMeta::Type::COMMAND);
    }

    template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<BlackboardBase, T>>>
    void operator()(T *&bb) {
        bb = new T{};
        module.blackboards.emplace_back(bb);
    }

    template<typename T, typename StateT>
    typename std::enable_if_t<not is_blob_v<T>> operator()(TaskOutput<T, StateT> &endpoint) {
        jsassert(not endpoint.connected);
        auto &link = context.get<TaskPool<T, StateT>>();
        endpoint.link = &link;
        addEndpoint(EndpointMeta::Direction::OUT, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
        endpoint.connected = true;
    }

    template<typename T, typename StateT, StateT ListenState>
    typename std::enable_if_t<not is_blob_v<T>> operator()(TaskInput<T, StateT, ListenState> &endpoint) {
        auto &link = context.get<TaskPool<T, StateT>>();
        endpoint.id = link.channel->listen(ListenState);
        endpoint.link = &link;
        addEndpoint(EndpointMeta::Direction::IN, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
    }

    ModuleId finish() {
        module.name = name;
        return meta.insertModule(module, endpoints);
    }
 
private:
    ContextPool &context;
    Metadata &meta;

    ModuleMeta module;
    std::vector<EndpointMeta> endpoints;

    void addEndpoint(
            EndpointMeta::Direction direction, bool required, void *obj, TypeID dataid, ChannelMeta::Type chanType) {
        EndpointMeta endpoint;
        endpoint.kind = direction;
        endpoint.required = required;
        endpoint.obj = obj;
        endpoint.channel = meta.findOrEmplaceChannel(dataid, chanType);
        endpoint.module = INVALID_ID;
        endpoint.id = INVALID_ID;
        endpoints.push_back(endpoint);
    }
};

} // namespace rt
