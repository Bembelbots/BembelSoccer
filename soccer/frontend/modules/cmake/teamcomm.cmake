set(MODTEAMCOMM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/teamcomm)

target_sources(libfrontend
PRIVATE
    ${MODTEAMCOMM_DIR}/teamcomm.cpp
)

add_library(modteamcomm INTERFACE)
