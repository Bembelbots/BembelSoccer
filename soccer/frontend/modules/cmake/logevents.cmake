set(MODLOGEVENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/logevents)

target_sources(libfrontend
PRIVATE
    ${MODLOGEVENTS_DIR}/logevents.cpp
)

add_library(modlogevents INTERFACE)
