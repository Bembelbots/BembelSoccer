#pragma once

#include "interpolation_function.hpp"
#include "linear.hpp"

#include <memory>


namespace joints {
namespace details {

template <class J>
class Interpolation {

public:

    Interpolation() {
        *this = Linear<J>();
    }

    Interpolation(const Interpolation<J> &other) {
        *this = *other.impl;
    }

    template<class Function>
    Interpolation<J> &operator=(const Function &func) {
        impl = std::make_unique<Function>(func);
        return *this;
    }

    J get(int time) {
        return impl->get(time);
    }

    bool done(int time) const {
        return impl->done(time);
    }

private:

    using Impl = std::unique_ptr<InterpolationFunction<J>>;

    Impl impl;
};

} // namespace details
} // namespace joints
