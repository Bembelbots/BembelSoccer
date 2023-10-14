add_subdirectory(backend/lola)

add_library(libbembelbots
    libbembelbots/config/botnames.cpp
)

target_include_directories(libbembelbots
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(libbembelbots bbrepr rt)
