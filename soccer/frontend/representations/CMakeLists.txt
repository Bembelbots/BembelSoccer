set(BBREPR_DIR  ${CMAKE_CURRENT_SOURCE_DIR})
set(BBREPR_FB_DIR  ${BBREPR_DIR}/flatbuffers)

set(FLATBUFFER_SCHEMAS
)

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

add_flatbuffers_library(
    bbrepr_flatbuffers
    SOURCES
        ${BBREPR_FB_DIR}/types/roles.fbs
        ${BBREPR_FB_DIR}/types/botnames.fbs
        ${BBREPR_FB_DIR}/types/lola_names.fbs
        ${BBREPR_FB_DIR}/types/robot_config.fbs
        ${BBREPR_FB_DIR}/types/actuators.fbs
        ${BBREPR_FB_DIR}/types/sensors.fbs
        ${BBREPR_FB_DIR}/types/campose.fbs
        ${BBREPR_FB_DIR}/types/gc_enums.fbs
        ${BBREPR_FB_DIR}/types/gc_teaminfo.fbs
        ${BBREPR_FB_DIR}/types/gc_robotinfo.fbs
        ${BBREPR_FB_DIR}/math/directed_coord.fbs
        ${BBREPR_FB_DIR}/math/coord.fbs
        ${BBREPR_FB_DIR}/messages/localization_message.fbs
        ${BBREPR_FB_DIR}/messages/referee_gesture_message.fbs
        ${BBREPR_FB_DIR}/messages/team_message.fbs
        ${BBREPR_FB_DIR}/messages/cam_pose_message.fbs
        ${BBREPR_FB_DIR}/messages/ipc_actuator_message.fbs
        ${BBREPR_FB_DIR}/messages/ipc_sensor_message.fbs
        ${BBREPR_FB_DIR}/messages/gamecontrol.fbs
        ${BBREPR_FB_DIR}/messages/gamestate_message.fbs
        ${BBREPR_FB_DIR}/messages/whistle_message.fbs
        ${BBREPR_FB_DIR}/messages/whistle_commands.fbs
        ${BBREPR_FB_DIR}/messages/log_event.fbs
        ${BBREPR_FB_DIR}/vision_results.fbs
    DESTINATION ${CMAKE_BINARY_DIR}/include/representations
)
target_sources(bbrepr_flatbuffers
PRIVATE
    ${BBREPR_FB_DIR}/flatbuffers.h
    ${BBREPR_FB_DIR}/types/directed_coord.cpp
    ${BBREPR_FB_DIR}/types/coord.cpp
    ${BBREPR_FB_DIR}/types/campose.cpp
    ${BBREPR_FB_DIR}/types/actuators.cpp
    ${BBREPR_FB_DIR}/types/sensors.cpp
)
target_link_libraries(bbrepr_flatbuffers bbframework)

add_library(bbrepr INTERFACE)
target_link_libraries(bbrepr INTERFACE bbrepr_flatbuffers bbframework libbembelbots Eigen3::Eigen)

add_executable(playingfield_debug EXCLUDE_FROM_ALL 
    ${BBREPR_DIR}/playingfield/playingfield_debug.cpp
    ${BBREPR_DIR}/playingfield/playingfield_painter.cpp
)
target_link_libraries(playingfield_debug
    libfrontend
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
