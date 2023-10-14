#pragma once

#include "meta.h"
#include "channels.h"
#include "endpoints.h"
#include "logdata/logdata.h"

#include <optional>

namespace rt {

class Kernel;

class Linker {

public:
    std::string name;

    Linker(ContextPool &context, Metadata &meta) : context(context), meta(meta) {}

    template<typename T, Flag Flags>
    void operator()(Input<T, Flags> &endpoint, enable_blob_t<T, false> = nullptr) {
        static_assert(flag_set(Flags, detail::InputFlags), "required input flag not set!");

        auto &link = context.get<MessageChannel<T>>();
        bool required = false;

        if constexpr (flag_set(Flags, Listen)) {
            endpoint.id = link.addListener();
            module.preProcess.emplace_back([&]() { link.maybeFetch(endpoint.id, endpoint.data); });
        } else if constexpr (flag_set(Flags, Require)) {
            required = true;
            endpoint.id = link.addListener();
            module.readyFuncs.emplace_back([&]() { return link.hasNewData(endpoint.id); });
            module.preProcess.emplace_back([&]() { link.fetch(endpoint.id, endpoint.data); });
        } else if constexpr (flag_set(Flags, Event)) {
            endpoint.id = link.addSnoopingListener();
            endpoint.link = &link;
        }

        addEndpoint(EndpointMeta::Direction::IN, required, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
    }
    
    template<typename T, Flag Flags>
    void operator()(Output<T, Flags> &endpoint, enable_blob_t<T, false> = nullptr) {
        static_assert(flag_set(Flags, detail::OutputFlags), "required output flag not set!");

        auto &link = context.get<MessageChannel<T>>();

        if constexpr (flag_set(endpoint.flags, Event)) {
            jsassert(endpoint.link == nullptr);
            endpoint.link = &link;
        } else if constexpr (flag_set(endpoint.flags, OutputFlag)) {
            jsassert(not endpoint.connected);
            module.postProcess.emplace_back([&]() { link.write(endpoint.data); });
            endpoint.connected = true;
        }

        if constexpr (flag_set(endpoint.flags, Flag::EnableLogging)) {
            if(not tag_set(module.tags, ModuleTag::DisableLogging)) {
                auto &logdata = context.get<LogDataContext>();
                size_t logid = logdata.addLogger(&module);
                logids.push_back(logid);
                link.addTap([&, logid](const T &data) {
                    auto &logger = logdata.at(logid);
                    logger.write(data);
                });
            }
        }

        addEndpoint(EndpointMeta::Direction::OUT, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::MESSAGE);
    }
    
    template<typename T, Flag Flags>
    void operator()(Context<T, Flags> &endpoint) {
        static_assert(flag_set(Flags, detail::ContextFlags), "required context flag not set!");

        if constexpr (std::is_same_v<T, LogDataContext>) {
            jsassert(tag_set(module.tags, ModuleTag::Logger)) << " not a logger!";
        }

        endpoint.data = &context.get<T>();

        auto direction = EndpointMeta::Direction::IN;

        if constexpr (flag_set(endpoint.flags, Read)) {
            // do nothing
        } else if constexpr (flag_set(endpoint.flags, Write)) {
            direction = EndpointMeta::Direction::OUT;
        }
        
        if constexpr (flag_set(endpoint.flags, Flag::EnableLogging)) {
            static_assert(std::is_same_v<T, std::false_type>, "not implemented!");
        }

        addEndpoint(direction, false, &endpoint, TypeInfo<T>::id(), ChannelMeta::Type::CONTEXT);
    }
    
    template<typename Group, Flag Flags>
    void operator()(Command<Group, Flags> &endpoint) {
        static_assert(flag_set(Flags, detail::CommandFlags), "required dispatch flag not set!");

        auto *link = &context.get<CommandChannel<Group>>();
        endpoint.link = link;
        
        auto direction = EndpointMeta::Direction::OUT;

        if constexpr (flag_set(endpoint.flags, Issue)) {
            // do nothing
        } else if constexpr (flag_set(endpoint.flags, Handle)) {
            direction = EndpointMeta::Direction::IN;
        }
        
        if constexpr (flag_set(endpoint.flags, Flag::EnableLogging)) {
            static_assert(std::is_same_v<Group, std::false_type>, "not implemented!");
        }
        
        addEndpoint(direction, false, &endpoint, TypeInfo<Group>::id(), ChannelMeta::Type::COMMAND);
    }
    
    template<typename ContextT, Flag Flags>
    void operator()(Task<ContextT, Flags> &endpoint) {
        static_assert(std::is_same_v<ContextT, std::false_type>, "not implemented!");
    }
    
    template<typename ContextT, typename T, Flag Flags>
    void operator()(TaskInput<ContextT, T, Flags> &endpoint) {
        static_assert(std::is_same_v<T, std::false_type>, "not implemented!");
    }
    
    template<typename ContextT, typename T, Flag Flags>
    void operator()(TaskOutput<ContextT, T, Flags> &endpoint) {
        static_assert(std::is_same_v<T, std::false_type>, "not implemented!");
    }

    template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<BlackboardBase, T>>>
    void operator()(T *&bb) {
        bb = new T{};
        module.blackboards.emplace_back(bb);
    }
    
    template<typename T, Flag Flags>
    void operator()(Input<T, Flags> &endpoint, enable_blob_t<T> = nullptr) {
        static_assert(std::is_same_v<T, std::false_type>, "not implemented!");
    }
    
    template<typename T, Flag Flags>
    void operator()(Output<T, Flags> &endpoint, enable_blob_t<T> = nullptr) {
        static_assert(std::is_same_v<T, std::false_type>, "not implemented!");
    }

    ModuleId finish() {
        module.name = name;
        ModuleId id = meta.insertModule(module, endpoints);
        auto &logdata = context.get<LogDataContext>();
        for(size_t logid : logids) {
            logdata.at(logid).id = id;
            //LOG_DEBUG << logdata.at(logid).id;
        }
        return id;
    }
 
private:
    friend Kernel;

    ContextPool &context;
    Metadata &meta;

    ModuleMeta module;
    std::vector<EndpointMeta> endpoints;

    std::vector<size_t> logids;

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
