cmake_minimum_required(VERSION 3.11)
project(BembelbotsFramework LANGUAGES CXX)

include(cmake/cflags.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_HEADERS_IN_COMPILE_COMMANDS ON)

# (skipped on jenkins, because of issues when running parallel builds)
if (NOT ${JENKINS})
    exec_program("git" ARGS "submodule update --init")
endif()

add_definitions("-DCPU_ONLY")
include(cmake/buildinfo.cmake)
include(cmake/bb-shared.cmake)
include(cmake/build-options.cmake)
include(cmake/detach_dbg_info.cmake)

if (${V6})
    find_package(Boost REQUIRED COMPONENTS system filesystem thread program_options chrono regex locale)
    find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)
endif (${V6})

add_subdirectory(external EXCLUDE_FROM_ALL)
add_subdirectory(firmware)
add_subdirectory(frontend)
