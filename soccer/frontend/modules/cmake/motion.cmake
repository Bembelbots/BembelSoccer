set(MODMOTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/motion)
set(BODYCONTROL_DIR ${MODMOTION_DIR}/bodycontrol)

target_sources(libfrontend
PRIVATE
    ${MODMOTION_DIR}/kinematics/body_chain.cpp
    ${MODMOTION_DIR}/kinematics/cam_pose.cpp
    ${MODMOTION_DIR}/kinematics/foot.cpp

    ${MODMOTION_DIR}/ahrs/ahrs.cpp
    ${MODMOTION_DIR}/ahrs/MadgwickAHRS.cpp

    ${MODMOTION_DIR}/walk/htwk/kinematics/ForwardKinematic.cpp
    ${MODMOTION_DIR}/walk/htwk/kinematics/InverseKinematics.cpp
    ${MODMOTION_DIR}/walk/htwk/kinematics/Pose3D.cpp
    ${MODMOTION_DIR}/walk/htwk/kinematics/PositionFeet.cpp
    
    ${MODMOTION_DIR}/walk/htwk/ankle_balancer.cpp
    ${MODMOTION_DIR}/walk/htwk/arm_controller.cpp
    
    ${MODMOTION_DIR}/walk/definitions/body.cpp

    ${MODMOTION_DIR}/walk/walk_calibration.cpp
    ${MODMOTION_DIR}/walk/walking_engine.cpp
    ${MODMOTION_DIR}/walk/htwkwalk.cpp

    ${BODYCONTROL_DIR}/bodycontrol.cpp

    ${BODYCONTROL_DIR}/blackboards/body_interface.cpp
    ${BODYCONTROL_DIR}/blackboards/kick_calibration.cpp
    ${BODYCONTROL_DIR}/blackboards/motion_design_blackboard.cpp

    ${BODYCONTROL_DIR}/utils/actuatorcheck.cpp
    ${BODYCONTROL_DIR}/utils/bbmf.cpp
    ${BODYCONTROL_DIR}/utils/motionfile.cpp
    ${BODYCONTROL_DIR}/utils/combined_balancer.cpp
    
    ${BODYCONTROL_DIR}/submodules/motion_design/motion_design_engine.cpp

    ${MODMOTION_DIR}/motion.cpp
)

add_library(modmotion INTERFACE)
target_include_directories(modmotion
INTERFACE
    ${MODMOTION_DIR}
)
target_link_libraries(modmotion INTERFACE Eigen3::Eigen)
