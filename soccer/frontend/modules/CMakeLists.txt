include(cmake/nao.cmake)
include(cmake/vision.cmake)
include(cmake/motion.cmake)
include(cmake/whistle.cmake)
include(cmake/behavior.cmake)
include(cmake/gamecontrol.cmake)
include(cmake/gamestate.cmake)
include(cmake/localization.cmake)
include(cmake/refereegesture.cmake)
include(cmake/teamcomm.cmake)
include(cmake/worldmodel.cmake)
include(cmake/logevents.cmake)

add_library(bbmodules INTERFACE)

target_sources(libfrontend
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/robocup.cpp
)

target_link_libraries(bbmodules
INTERFACE
    bbframework
    bbrepr
    modnao
    modvision
    modmotion
    modwhistle
    modbehavior
    modgamecontrol
    modgamestate
    modlocalization
    modrefereegesture
    modteamcomm
    modworldmodel
    modlogevents
)
