#pragma once

#include "endpoint.h"

namespace rt {

class Linker;

namespace  detail {
    static constexpr Flag OutputFlags = OutputFlag | Event;

    static constexpr Flag getOutputFlags(Flag flags) {
        if (not flag_set(flags, rt::DisableLogging)) {
            return flags | EnableLogging;
        }
        return flags;
    }
    
    template<typename T, Flag Flags>
    class Output {
        static_assert(std::is_same_v<decltype(Flags), std::false_type>, "unsupported flag!");
    };

    template<typename T>
    class Output<T, OutputFlag> {
    public:
        Output() = default;

        inline T &operator*() { return assure(); }
        inline T *operator->() { return &assure(); }
        inline operator T &() { return assure(); }
        inline operator T *() { return &assure(); }

    private:
        friend class rt::Linker;

        T data = {};
        bool connected = false;

        T &assure() {
            jsassert(connected);
            return data;
        }
    };

    template<typename T>
    class Output<T, Event> {
    public:
        Output() = default;

        void emit(T &data) { assertLink().write(data); }
        void emit(T &&data) { assertLink().write(data); }

    private:
        friend class rt::Linker;
        
        MessageChannel<T> *link = nullptr;
     
        auto &assertLink() {
            jsassert(link != nullptr);
            return *link;
        }
    };
}

template<typename T, Flag Flags = OutputFlag>
struct Output : public detail::Output<T, Flags & detail::OutputFlags> {
    static constexpr Flag flags = detail::getOutputFlags(Flags);
};

} // namespace rt
