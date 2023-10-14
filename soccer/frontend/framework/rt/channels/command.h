#pragma once

#include "../message_utils.h"

#include <entt/signal/dispatcher.hpp>
#include <mutex>

namespace rt {

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

} // namespace rt
