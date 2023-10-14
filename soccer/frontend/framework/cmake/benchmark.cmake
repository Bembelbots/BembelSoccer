set(BBBENCHMARK_PATH ${BBFRAMEWORK_PATH}/benchmark)

target_sources(bbframework
PRIVATE
    ${BBBENCHMARK_PATH}/timer.cpp
)

add_library(bbbenchmark INTERFACE)
target_compile_features(bbbenchmark INTERFACE cxx_std_17)
