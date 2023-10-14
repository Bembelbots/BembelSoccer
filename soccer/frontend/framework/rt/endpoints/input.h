#pragma once

#include "endpoint.h"

namespace rt {

class Linker;

static constexpr Flag Listen    = OptionalInput;
static constexpr Flag Snoop     = Event;

namespace  detail {
    static constexpr Flag InputFlags = Listen | Snoop | Event | Require;
    
    static constexpr Flag getInputFlags(Flag flags) {
        /* uncomment to make logging default
        if (not flag_set(flags, rt::DisableLogging)) {
            return flags | EnableLogging;
        }
        */
        return flags;
    }

    template<typename T, Flag Flags>
    class Input {
        static_assert(std::is_same_v<decltype(Flags), std::false_type>, "unsupported flag!");
    };

    template<typename T>
    class Input<T, Listen> {
    public:
        Input() = default;

        inline const T &operator*() const { return assure(); }
        inline const T *operator->() const { return &assure(); }
        inline operator const T &() const { return assure(); }
        inline operator const T *() const { return &assure(); }

    private:
        friend class rt::Linker;

        T data = {};
        int id = -1;

        inline bool connected() const { return id != -1; }

        const T &assure() const {
            jsassert(connected());
            return data;
        }
    };
    
    template<typename T>
    class Input<T, Require> : public Input<T, Listen> {};

    template<typename T>
    class Input<T, Event> {

    public:
        std::vector<T> fetch() { return assertLink().snoopFetch(id); }
        void waitWhileEmpty() { assertLink().waitWhileEmpty(id); }

    private:
        friend class rt::Linker;

        int id = -1;
        MessageChannel<T> *link = nullptr;

        auto &assertLink() {
            jsassert(link != nullptr && id != -1);
            return *link;
        }
    };
}

template<typename T, Flag Flags = Listen>
struct Input : public detail::Input<T, Flags & detail::InputFlags> {
    static constexpr Flag flags = detail::getInputFlags(Flags);
};

} // namespace rt
