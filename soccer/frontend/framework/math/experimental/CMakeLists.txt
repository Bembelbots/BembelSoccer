set(MATHLIB_SRCS
        "coord.cpp"
        "radians.cpp"
        "degrees.cpp"
        "angle.cpp"
        "linesegment.h"
        "line.h"
        "circle.h"
        "algorithms/DBScan.cpp")

add_library(mathlib STATIC EXCLUDE_FROM_ALL ${MATHLIB_SRCS})
