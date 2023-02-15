set(BBCOMMON_PATH ${BBFRAMEWORK_PATH}/common)

target_sources(bbframework
PRIVATE
    ${BBCOMMON_PATH}/platform.cpp
)

add_library(bbcommon INTERFACE)
target_compile_features(bbcommon INTERFACE cxx_std_17)