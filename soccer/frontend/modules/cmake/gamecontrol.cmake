set(MODGAMECONTROL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gamecontrol)

target_sources(libfrontend
PRIVATE
    ${MODGAMECONTROL_DIR}/gamecontrol.cpp
)

add_library(modgamecontrol INTERFACE)
