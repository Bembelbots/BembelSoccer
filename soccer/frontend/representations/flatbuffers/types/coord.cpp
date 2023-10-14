#include "coord.h"

namespace flatbuffers {
    bbapi::Coord Pack(const Coord& dc) {
        return bbapi::Coord(dc.x, dc.y);
    }

    Coord UnPack(const bbapi::Coord& dc) {
        return Coord(dc.x(), dc.y());
    }
}
