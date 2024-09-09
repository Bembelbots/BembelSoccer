#pragma once

#include "util/type_info.h"
#include "module_tags.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <limits>

class BlackboardBase;

namespace rt {

using ModuleId = size_t;
using EndpointId = size_t;
using ChannelId = size_t;

inline constexpr size_t INVALID_ID = std::numeric_limits<size_t>::max();

struct EndpointMeta {

    enum class Direction { IN, OUT };

    Direction kind;
    ChannelId channel;
    bool required;
    void *obj;

    ModuleId module;
    EndpointId id;
};

struct ChannelMeta {
    enum class Type { COMMAND, MESSAGE, CONTEXT, BLOB };

    Type kind;
    TypeID dataType;
    ChannelId id;
    std::vector<EndpointId> endpoints;
    void *obj;
};

struct ModuleMeta {

    using CleanupFunc = std::function<void()>;
    using UpdateFunc = std::function<void()>;
    using Requirement = std::function<bool()>;

    std::vector<Requirement> readyFuncs;
    std::vector<UpdateFunc> preProcess;
    std::vector<UpdateFunc> postProcess;

    std::string name;
    ModuleTag tags = ModuleTag::None;
    ModuleId id = INVALID_ID;

    std::vector<EndpointId> endpoints;
    std::vector<std::shared_ptr<BlackboardBase>> blackboards;
    std::vector<ModuleId> requiredBy;

    bool ready() const;

    void doPreProcess() const;
    void doPostProcess() const;
};

struct Metadata {

    static std::string toStr(ChannelMeta::Type);
    static std::string toStr(EndpointMeta::Direction);

    std::vector<EndpointMeta> endpoints;
    std::vector<ChannelMeta> channels;
    std::vector<ModuleMeta> modules;

    void setRequiredBy();

    ModuleId insertModule(ModuleMeta &, std::vector<EndpointMeta> &);
    ChannelId findOrEmplaceChannel(TypeID, ChannelMeta::Type);

    EndpointId firstIn(ChannelId) const;
    EndpointId firstOut(ChannelId) const;

    std::string dumpGraph() const;
    std::string printModule(const ModuleMeta &module) const;
    
    std::string channelError(ChannelId id, std::string_view errorMsg);
    std::string moduleError(ModuleId id, std::string_view errorMsg);
};

} // namespace rt
