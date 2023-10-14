set(BBSERIALIZE_PATH ${BBFRAMEWORK_PATH}/serialize)

target_sources(bbframework
PRIVATE
    ${BBSERIALIZE_PATH}/serializer.cpp
)

add_library(bbserialize INTERFACE)
target_compile_features(bbserialize INTERFACE cxx_std_17)