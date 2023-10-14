#pragma once

#include "../../logger/logger.h"
#include "../util/type_info.h"

#include <flatbuffers/flatbuffers.h>
#include <type_traits>

namespace  rt {

using LogDataSerializedType = std::tuple<size_t, uint8_t*>;

template<typename T>
struct LogDataSerializer {
    flatbuffers::FlatBufferBuilder builder;

    static constexpr bool is_serializable = std::is_base_of<flatbuffers::NativeTable, T>::value;

    LogDataSerializedType serialize(const T &data) {
        if constexpr (is_serializable) {
            builder.Reset();
            builder.Finish(T::TableType::Pack(builder, &data));
            return std::make_tuple(builder.GetSize(), builder.GetBufferPointer());
        } else {
            LOG_INFO_FIRST_N(1) << "trying to serialize a non serializable type " << prettyTypeName(TypeInfo<T>::id());
            return std::make_tuple(0, nullptr);
        }
    }
};

#define LOG_SERIALIZE(TYPE, ...) \
template<> \
struct LogDataSerializer<TYPE> { \
    LogDataSerializedType serialize(const T &data) \
    __VA_ARGS__ \
};

} // namespace rt
