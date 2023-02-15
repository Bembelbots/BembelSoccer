#pragma once
#include <cstdint>

// type to keep unix stamp in microseconds
typedef int64_t microTime;

// type to keep unix stamp in milliseconds
typedef int32_t TimestampMs;

// get unix time-stamp in microseconds (1e-6 seconds)
microTime getMicroTime();

// get timestamp in milliseconds (1e-3 seconds)
TimestampMs getTimestampMs();
