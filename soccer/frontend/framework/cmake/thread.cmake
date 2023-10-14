set(BBTHREAD_PATH ${BBFRAMEWORK_PATH}/thread)

target_sources(bbframework
PRIVATE
    ${BBTHREAD_PATH}/util.cpp
)

add_library(bbthread INTERFACE)
target_compile_features(bbthread INTERFACE cxx_std_17)
target_link_libraries(bbthread INTERFACE pthread)
