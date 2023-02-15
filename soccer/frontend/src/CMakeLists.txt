set(EXECUTABLE_NAME jsfrontend)

add_executable(${EXECUTABLE_NAME}
    main.cpp
    threadconfig.cpp
    system.cpp
    config.cpp
    network/debugserver.cpp
)
add_buildinfo(${EXECUTABLE_NAME})
detach_dbg_info(${EXECUTABLE_NAME})

target_include_directories(${EXECUTABLE_NAME}
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC libfrontend boost_program_options ${Boost_THREAD_LIBRARY})
target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_17)
