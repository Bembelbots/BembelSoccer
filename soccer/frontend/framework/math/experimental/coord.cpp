#include "coord.h"

namespace MathToolbox {

std::ostream &operator<<(std::ostream &os, const CoordTypes &coord_type) {
    std::string type = "";

    switch (coord_type) {
        case RCS:
            type = "RCS";
            break;
        case WCS:
            type = "WCS";
            break;
        case MCS:
            type = "MCS";
            break;
        case ICS:
            type = "ICS";
            break;
        case CCS:
            type = "CCS";
            break;
        default:
            type = "UNKNOWN";
            break;
    }

    return os << type;
}

} // namespace MathToolbox