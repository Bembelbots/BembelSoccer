#include "directed_coord.h"

namespace flatbuffers {
    bbapi::DirectedCoord Pack(const DirectedCoordNative& dc) {
        return bbapi::DirectedCoord(dc.x(), dc.y(), dc.rad());
    }

    DirectedCoordNative UnPack(const bbapi::DirectedCoord& dc) {
        return DirectedCoordNative(dc.x(), dc.y(), Rad { dc.alpha() });
    }
}
