#pragma once

#include <chrono>

using sysMicroTime = long long int; //< Timestamp in microseconds from the systemclock

inline sysMicroTime getSystemTimestamp() {
    using namespace std::chrono;
    // WARNING: do not change clock! Other clocks than steady_clock will not match camera timestamps and break things!
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}


