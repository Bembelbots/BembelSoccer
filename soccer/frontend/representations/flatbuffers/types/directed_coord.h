#pragma once
#include <framework/math/directed_coord.h>
#include <directed_coord_generated.h>

using DirectedCoordNative = DirectedCoord;

namespace flatbuffers {
    bbapi::DirectedCoord Pack(const DirectedCoordNative& dc);
    DirectedCoordNative UnPack(const bbapi::DirectedCoord& dc);
}
