add_library(bbframework)

include(${BBFRAMEWORK_CMAKE_PATH}/common.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/util.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/logger.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/math.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/runtime.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/blackboard.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/image.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/network.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/serialize.cmake)
include(${BBFRAMEWORK_CMAKE_PATH}/benchmark.cmake)

target_include_directories(bbframework
PUBLIC
    ${BBFRAMEWORK_PATH}/..
PRIVATE
    ${BBFRAMEWORK_PATH}
)

target_link_libraries(bbframework
PUBLIC
    bbcommon
    bbutil
    bblogger
    bbmath
    bbruntime
    bbimage
    bbnetwork
    bbserialize
    bbbenchmark
    ${OpenCV_LIBS}
)
add_dependencies(bbframework git_submodules)
