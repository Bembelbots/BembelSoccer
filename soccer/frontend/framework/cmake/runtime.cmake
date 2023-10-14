set(BBRUNTIME_PATH ${BBFRAMEWORK_PATH}/rt)

target_sources(bbframework
PRIVATE
    ${BBRUNTIME_PATH}/depth_first_search.cpp
    ${BBRUNTIME_PATH}/kernel.cpp
    ${BBRUNTIME_PATH}/meta.cpp
    ${BBRUNTIME_PATH}/type_info.cpp
)

add_library(bbruntime INTERFACE)
target_compile_features(bbruntime INTERFACE cxx_std_17)
target_link_libraries(bbruntime INTERFACE gsl entt)
