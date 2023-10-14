#pragma once

#include "endpoint.h"

namespace rt {

class Linker;

static constexpr Flag Handle    = OptionalInput;
static constexpr Flag Issue     = OutputFlag;

namespace detail {
    static constexpr Flag CommandFlags = Issue | Handle;
    
    static constexpr Flag getCommandFlags(Flag flags) {
        /* uncomment to make logging default
        if (not flag_set(flags, rt::DisableLogging)) {
            return flags | EnableLogging;
        }
        */
        return flags;
    }

    template<typename Group, Flag Flags>
    struct Command {
        static_assert(std::is_same_v<decltype(Flags), std::false_type>, "unsupported flag!");
    };

    template<typename Group>
    struct Command<Group, Issue> {
    public:
        template<typename Command, typename = typename std::enable_if_t<is_command_v<Command, Group>>, typename... Args>
        void enqueue(Args &&... args) {
            link->template enqueue<Command>(std::forward<Args>(args)...);
        }

    private:
        friend class rt::Linker;
        CommandChannel<Group> *link = nullptr;
    };

    template<typename Group>
    struct Command<Group, Handle> {

        template<typename Command, auto Candidate, typename = typename std::enable_if_t<is_command_v<Command, Group>>>
        void connect() {
            link->template connect<Command, Candidate>();
        }

        template<typename Command, auto Candidate, typename Type,
                typename = typename std::enable_if_t<is_command_v<Command, Group>>>
        void connect(Type &&valueOrInstance) {
            link->template connect<Command, Candidate, Type>(std::forward<Type>(valueOrInstance));
        }

        void update() { link->update(); }

    private:
        friend class rt::Linker;
        CommandChannel<Group> *link = nullptr;
    };
}

template<typename T, Flag Flags = Issue>
struct Command : public detail::Command<T, Flags & detail::CommandFlags> {
    static constexpr Flag flags = detail::getCommandFlags(Flags);
};

} // namespace rt
