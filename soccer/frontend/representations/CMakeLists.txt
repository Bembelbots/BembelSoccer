set(BBREPR_DIR  ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(libfrontend
PRIVATE
    ${BBREPR_DIR}/blackboards/camera_calibration.cpp
    ${BBREPR_DIR}/blackboards/settings.cpp
    ${BBREPR_DIR}/blackboards/camera_parameters.cpp
    ${BBREPR_DIR}/blackboards/vision.cpp
    ${BBREPR_DIR}/blackboards/gamecontrol.cpp
    ${BBREPR_DIR}/blackboards/worldmodel.cpp

    ${BBREPR_DIR}/bembelbots/nao_info.cpp

    ${BBREPR_DIR}/vision/visiondefinitions.cpp

    ${BBREPR_DIR}/serialize/eigen.h

    ${BBREPR_DIR}/playingfield/playingfield.cpp

    ${BBREPR_DIR}/worldmodel/definitions.cpp
)

add_library(bbrepr INTERFACE)
target_link_libraries(bbrepr INTERFACE bbframework libbembelbots Eigen)

add_executable(playingfield_debug EXCLUDE_FROM_ALL 
	${BBREPR_DIR}/playingfield/playingfield_debug.cpp
	${BBREPR_DIR}/playingfield/playingfield_painter.cpp
)
target_link_libraries(playingfield_debug
	libfrontend
	${Boost_PROGRAM_OPTIONS_LIBRARY}
)
