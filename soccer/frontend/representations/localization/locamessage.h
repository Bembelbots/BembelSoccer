#pragma once
#include <framework/math/directed_coord.h>
#include <framework/util/clock.h>

struct LocalizationMessage {
    DirectedCoord pose;
    float confidence;
    TimestampMs gtTimestamp;
    DirectedCoord gtPosition;
};
