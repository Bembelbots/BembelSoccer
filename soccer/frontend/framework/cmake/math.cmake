set(BBMATH_PATH ${BBFRAMEWORK_PATH}/math)
set(BBMATH_EXP_PATH ${BBMATH_PATH}/experimental)

target_sources(bbframework
PRIVATE
    ${BBMATH_PATH}/angle.cpp
    ${BBMATH_PATH}/coord.cpp
    ${BBMATH_PATH}/directed_coord.cpp
    ${BBMATH_PATH}/rotation_matrices.cpp
    ${BBMATH_PATH}/point.hpp
    ${BBMATH_PATH}/old.cpp
    
    ${BBMATH_EXP_PATH}/coord.cpp
    ${BBMATH_EXP_PATH}/radians.cpp
    ${BBMATH_EXP_PATH}/degrees.cpp
    ${BBMATH_EXP_PATH}/angle.cpp
    ${BBMATH_EXP_PATH}/linesegment.h
    ${BBMATH_EXP_PATH}/line.h
    ${BBMATH_EXP_PATH}/circle.h
    ${BBMATH_EXP_PATH}/algorithms/DBScan.cpp
)

add_library(bbmath INTERFACE)
target_compile_features(bbmath INTERFACE cxx_std_17)
target_link_libraries(bbmath INTERFACE Eigen3::Eigen)
