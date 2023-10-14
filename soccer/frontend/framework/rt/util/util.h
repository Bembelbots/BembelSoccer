#pragma once

#include "../meta.h"

#define RT_DISABLE_COPY(CLASS) \
    CLASS(const CLASS &) = delete; \
    CLASS(CLASS &&) = delete; \
    CLASS &operator=(const CLASS &) = delete; \
    CLASS &operator=(CLASS &&) = delete;

namespace  rt {

std::string composeError(std::string_view prefix, std::string_view errorMsg);

} // namespace rt
