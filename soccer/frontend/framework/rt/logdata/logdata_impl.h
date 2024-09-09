#pragma once

#include "serializer.h"
#include "../util/type_info.h"
#include <memory>

namespace  rt {

struct LogDataAny {};

struct LogDataInterface {
    virtual ~LogDataInterface() = default;
    virtual TypeID id() = 0;
    virtual std::string name() = 0;
    virtual bool is_serializeable() = 0;
    virtual LogDataSerializedType serialize() = 0;
};

template<typename  T>
struct LogDataContainer : public LogDataInterface {
    LogDataSerializer<T> serializer;
    T data;
    
    LogDataContainer(T &data)
        : data(std::move(data)) {
    }

    T* operator->() {
        return &data;
    }
    
    const T* operator->() const {
        return &data;
    }

    TypeID id() override {
        return TypeInfo<T>::id();
    }
    
    std::string name() override {
        return prettyTypeName(id());
    }

    bool is_serializeable() override {
        return serializer.is_serializable;
    }

    LogDataSerializedType serialize() override {
        return serializer.serialize(data);
    };
};

class LogData {
public:
    LogData() = default;

    template<typename T>
    LogData(T data)
        : storage(new LogDataContainer<T>(data)) {

    }

    LogDataInterface* operator->() {
        return storage.get();
    }
    
    const LogDataInterface* operator->() const {
        return storage.get();
    }

    template<typename T>
    std::optional<LogDataContainer<T>*> get() {
        if(storage->id() == TypeInfo<T>::id()) {
            return static_cast<LogDataContainer<T>*>(storage.get());
        }
        return {};
    }

    template<typename T, typename Functor, typename ...Args>
    bool handle(Functor fn, Args && ...args) {
        if(handled) {
            return false;
        }

        if constexpr (std::is_same_v<LogDataAny, T>) {
            fn(std::forward<Args>(args)..., *this);
            handled = true;
            return true;
        } else {
            auto inst = get<T>();
            if(inst) {
                fn(std::forward<Args>(args)..., *inst.value());
                handled = true;
                return true;
            }
            return false;
        }        
    }

    bool is_handled() const {
        return handled;
    }

private:
    std::shared_ptr<LogDataInterface> storage;

    bool handled = false;
};

} // namespace rt
