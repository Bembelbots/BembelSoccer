#pragma once
#include <framework/math/coord.h>
#include <coord_generated.h>

using CoordNative = Coord;

namespace flatbuffers {
    bbapi::Coord Pack(const CoordNative& dc);
    CoordNative UnPack(const bbapi::Coord& dc);
}
