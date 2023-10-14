
set(MODWORLDMODEL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/worldmodel)

target_sources(libfrontend
PRIVATE
    ${MODWORLDMODEL_DIR}/worldmodel.cpp
    ${MODWORLDMODEL_DIR}/teamball.cpp
    ${MODWORLDMODEL_DIR}/ballmotionfilter.cpp
    ${MODWORLDMODEL_DIR}/ballfilter.cpp
)

add_library(modworldmodel INTERFACE)
