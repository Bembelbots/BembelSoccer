#pragma once

#include "endpoint.h"

namespace rt {

static constexpr Flag Read      = OptionalInput;
static constexpr Flag Write     = OutputFlag;

namespace detail {    
    static constexpr Flag ContextFlags = Read | Write;
    
    static constexpr Flag getContextFlags(Flag flags) {
        /* uncomment to make logging default
        if (not flag_set(flags, rt::DisableLogging)) {
            return flags | EnableLogging;
        }
        */
        return flags;
    }

    template<typename T, Flag Flags>
    struct Context {
        static_assert(std::is_same_v<decltype(Flags), std::false_type>, "unsupported flag!");
    };

    template<typename T>
    struct Context<T, Read> : public Endpoint<const T> {};

    template<typename T>
    struct Context<T, Write> : public Endpoint<T> {};
}

template<typename T, Flag Flags = Read>
struct Context : public detail::Context<T, Flags & detail::ContextFlags> {
    static constexpr Flag flags = detail::getContextFlags(Flags);
};

} // namespace rt
