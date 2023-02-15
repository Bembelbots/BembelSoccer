add_library(libfrontend)

add_subdirectory(framework)
add_subdirectory(modules)
add_subdirectory(representations)

target_include_directories(libfrontend
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(libfrontend bbmodules bbrepr)

add_subdirectory(src)
