#include "meta.h"
#include "util/util.h"

#include "../util/assert.h"

using namespace rt;

bool ModuleMeta::ready() const {
    bool ready = true;
    for (auto &f : readyFuncs) {
        ready &= f();
    }
    return ready;
}

void ModuleMeta::doPreProcess() {
    for (auto &func : preProcess) {
        func();
    }
}

void ModuleMeta::doPostProcess() {
    for (auto &func : postProcess) {
        func();
    }
}

std::string Metadata::toStr(ChannelMeta::Type t) {
    switch (t) {
        case ChannelMeta::Type::CONTEXT:
            return "context";
        case ChannelMeta::Type::COMMAND:
            return "command";
        case ChannelMeta::Type::BLOB:
            return "blob";
        case ChannelMeta::Type::MESSAGE:
            return "message";
    }

    JS_UNREACHABLE();
}

std::string Metadata::toStr(EndpointMeta::Direction dir) {
    switch (dir) {
        case EndpointMeta::Direction::IN:
            return "in";
        case EndpointMeta::Direction::OUT:
            return "out";
    }

    JS_UNREACHABLE();
}

void Metadata::setRequiredBy() {
    for (auto &module : modules) {
        std::vector<ModuleId> requiredBy;
        for (auto &endpointId : module.endpoints) {
            auto &point = endpoints[endpointId];
            auto &chan = channels[point.channel];
            if (point.kind == EndpointMeta::Direction::IN || chan.kind != ChannelMeta::Type::MESSAGE) {
                continue;
            }
            for (auto &chanEndpointId : chan.endpoints) {
                auto &other = endpoints[chanEndpointId];
                if (other.kind == EndpointMeta::Direction::OUT) {
                    continue;
                }
                if (other.required) {
                    requiredBy.emplace_back(other.module);
                }
            }
        }
        module.requiredBy = std::move(requiredBy);
    }
}

ModuleId Metadata::insertModule(ModuleMeta &module, std::vector<EndpointMeta> &endpoints) {
    jsassert(module.name != "");
    module.id = modules.size();

    jsassert(module.endpoints.empty());

    for (size_t i = 0; i < endpoints.size(); i++) {
        endpoints[i].id = this->endpoints.size();
        endpoints[i].module = module.id;
        module.endpoints.push_back(endpoints[i].id);
        channels.at(endpoints[i].channel).endpoints.push_back(endpoints[i].id);
        this->endpoints.emplace_back(std::move(endpoints[i]));
    }

    modules.emplace_back(std::move(module));

    return modules.back().id;
}

ChannelId Metadata::findOrEmplaceChannel(TypeID dataType, ChannelMeta::Type kind) {
    for (auto &chan : channels) {
        if (chan.dataType == dataType) {
            jsassert(chan.kind == kind);
            return chan.id;
        }
    }

    ChannelMeta chan{
            .kind = kind,
            .dataType = dataType,
            .id = channels.size(),
            .endpoints = {},
            .obj = nullptr,
    };
    channels.push_back(chan);
    return channels.back().id;
}

EndpointId Metadata::firstIn(ChannelId channelId) const {
    for (auto endpointId : channels.at(channelId).endpoints) {
        if (endpoints[endpointId].kind == EndpointMeta::Direction::IN) {
            return endpointId;
        }
    }
    return INVALID_ID;
}

EndpointId Metadata::firstOut(ChannelId channelId) const {
    for (auto endpointId : channels.at(channelId).endpoints) {
        if (endpoints[endpointId].kind == EndpointMeta::Direction::OUT) {
            return endpointId;
        }
    }
    return INVALID_ID;
}

std::string Metadata::channelError(ChannelId id, std::string_view errorMsg) {
    auto &chan = channels.at(id);
    std::string messageName = prettyTypeName(chan.dataType);

    std::string messageType = "<<< UNKNOW MESSAGE TYPE >>>";
    switch (chan.kind) {
        case ChannelMeta::Type::BLOB:
        case ChannelMeta::Type::MESSAGE:
            messageType = "Message";
            break;
        case ChannelMeta::Type::COMMAND:
            messageType = "Commands of type";
            break;
        case ChannelMeta::Type::CONTEXT:
            messageType = "Context";
            break;
    }

    return composeError(messageType + " '" + messageName + "'", errorMsg);
}

std::string Metadata::moduleError(ModuleId id, std::string_view errorMsg) {
    return composeError("Module '" + modules.at(id).name + "'", errorMsg);
}

std::string Metadata::dumpGraph() const {

    std::stringstream ss{};

    ss << "MODULES" << std::endl;
    for (auto &module : modules) {
        ss << "  id = " << module.id << std::endl << "  name = " << module.name << std::endl << std::endl;
    }

    ss << "CHANNELS" << std::endl;
    for (auto &chan : channels) {
        ss << "  id = " << chan.id << std::endl
           << "  kind = " << toStr(chan.kind) << std::endl
           << "  dataType = " << prettyTypeName(chan.dataType) << std::endl;

        ss << "  endpoints = [ ";
        for (auto id : chan.endpoints) {
            ss << id << ", ";
        }
        ss << " ]" << std::endl;
        ss << std::endl;
    }

    ss << "ENDPOINTS" << std::endl;
    for (auto &endpoint : endpoints) {
        ss << "  id = " << endpoint.id << std::endl
           << "  kind = " << toStr(endpoint.kind) << std::endl
           << "  module = " << endpoint.module << std::endl
           << "  channel = " << endpoint.channel << std::endl
           << "  required = " << endpoint.required << std::endl
           << std::endl;
    }

    return ss.str();
}

std::string Metadata::printModule(const ModuleMeta &module) const {
    std::stringstream ss{};

    ss << module.name << " [id = " << module.id << "]" << std::endl;

    for (EndpointId endpointId : module.endpoints) {
        const EndpointMeta &endpoint = endpoints.at(endpointId);
        const ChannelMeta &chan = channels.at(endpoint.channel);

        ss << "  ";

        switch (chan.kind) {
            case ChannelMeta::Type::COMMAND:
                if (endpoint.kind == EndpointMeta::Direction::IN) {
                    ss << "HANDLES";
                } else {
                    ss << "ISSUES";
                }
                ss << " " << prettyTypeName(chan.dataType);

                if (endpoint.kind == EndpointMeta::Direction::OUT) {
                    ss << " -> " << modules.at(endpoints.at(firstIn(chan.id)).module).name;
                }
                break;

            case ChannelMeta::Type::CONTEXT:
                if (endpoint.kind == EndpointMeta::Direction::IN) {
                    ss << "READS CONTEXT";
                } else {
                    ss << "WRITES CONTEXT";
                }
                ss << " " << prettyTypeName(chan.dataType);

                if (endpoint.kind == EndpointMeta::Direction::IN) {
                    ss << " <- " << modules.at(endpoints.at(firstOut(chan.id)).module).name;
                }
                break;

            case ChannelMeta::Type::BLOB:
            case ChannelMeta::Type::MESSAGE:
                if (endpoint.kind == EndpointMeta::Direction::IN) {
                    if (endpoint.required) {
                        ss << "REQUIRES";
                    } else {
                        ss << "LISTENS";
                    }
                }
                else {
                    ss << "PROVIDES";
                }
                ss << " " << prettyTypeName(chan.dataType);

                if (endpoint.kind == EndpointMeta::Direction::IN) {
                    ss << " <- " << modules.at(endpoints.at(firstOut(chan.id)).module).name;
                }
                break;
        }

        ss << std::endl;
    }

    for (auto r : module.requiredBy) {
        ss << "  "
           << "REQUIRED BY " << modules[r].name << std::endl;
    }

    return ss.str();
}
