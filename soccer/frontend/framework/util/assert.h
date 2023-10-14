#pragma once

#include "../logger/logger.h"
#include "../common/macros.h"

#include <string>
#include <string_view>

std::string jsassertMsg(std::string_view, int, std::string_view);
std::string jsunreachableMsg(std::string_view, int);
void jsassertAbort() ATTR_NO_RETURN;

// clang-format off
#define jsassert(expr) \
    if constexpr(BUILDING_DEBUG) \
        for (; UNLIKELY(not (expr)); jsassertAbort()) \
            LOG_ERROR << jsassertMsg(__FILE__, __LINE__, #expr)

#define JS_UNREACHABLE() \
    if constexpr(BUILDING_DEBUG) \
        for (; true; jsassertAbort()) \
            LOG_ERROR << jsunreachableMsg(__FILE__, __LINE__)
// clang-format on
