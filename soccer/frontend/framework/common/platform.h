#pragma once

//#include <shared/common/types.h>
#include <string>
#include "../util/clock.h"

// get tmp dir, including trailing slash or backslash
std::string getTmpDir();

// get a string containing a backtrace (gnu only)
std::string getBacktrace();


// vim: set ts=4 sw=4 sts=4 expandtab:
