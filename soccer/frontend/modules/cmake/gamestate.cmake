set(MODGAMESTATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gamestate)

target_sources(libfrontend
PRIVATE
    ${MODGAMESTATE_DIR}/gamestate.cpp
)

add_library(modgamestate INTERFACE)
