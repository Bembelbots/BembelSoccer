set(MODLOCALIZIATION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/localization)

target_sources(libfrontend
PRIVATE
    ${MODLOCALIZIATION_DIR}/poseblackboard.cpp
    ${MODLOCALIZIATION_DIR}/pose.cpp
    ${MODLOCALIZIATION_DIR}/particlefilter.cpp
    ${MODLOCALIZIATION_DIR}/hypothesesgenerator.cpp
)

add_library(modlocalization INTERFACE)
