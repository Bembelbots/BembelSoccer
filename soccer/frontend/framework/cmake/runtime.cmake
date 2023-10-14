set(BBRUNTIME_PATH ${BBFRAMEWORK_PATH}/rt)

target_sources(bbframework
PRIVATE
    ${BBRUNTIME_PATH}/kernel.cpp
    ${BBRUNTIME_PATH}/meta.cpp
    ${BBRUNTIME_PATH}/util/util.cpp
    ${BBRUNTIME_PATH}/util/type_info.cpp
    ${BBRUNTIME_PATH}/util/depth_first_search.cpp
)

add_library(bbruntime INTERFACE)
target_compile_features(bbruntime INTERFACE cxx_std_17)
target_link_libraries(bbruntime INTERFACE gsl entt flatbuffers)
