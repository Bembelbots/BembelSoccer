#include "motion.h"
#include <framework/rt/kernel.h>
#include <framework/logger/logger.h>
#include <modules/nao/nao.h>

#include "ahrs/ahrs.h"
#include "kinematics/cam_pose.h"
#include "bodycontrol/bodycontrol.h"
#include "bodycontrol/internals/submodule.h"
#include "bodycontrol/utils/bbmf.h"
#include "bodycontrol/utils/motionfile.h"

void MotionModule::load(rt::Kernel &soccer) {
    auto nao_ptr = new Nao();
    soccer.load(nao_ptr);
    nao_ptr->set_sensor_callback(std::bind(&MotionModule::tick, this, std::placeholders::_1));
    nao = std::shared_ptr<rt::ModuleLoader>(nao_ptr);
}

void MotionModule::connect(rt::Linker &link) {
    link(settings);
    link(body_state);
    link(cmds);
    bc = std::make_shared<BodyControl>();
    bc->connect(link);
}

void MotionModule::setup() {
    BBMF::bbmf_path = settings->motionsPath;
    MotionFile::motion_path = BBMF::bbmf_path;
    SubModule::Setup bcSetup{settings, &cmds};
    bc->setup(bcSetup);
}

void MotionModule::tick(NaoState &state) {
    auto bc_state = bc->step(cmds, state.actuatorData, state.sensorData);
    interface.update(bc_state);
    body_state.emit(bc_state);
    state.tCamPose = bc_state.tCamPose;
    state.bCamPose = bc_state.bCamPose;
    LOG_DEBUG_EVERY_N(84 * 10) << "Nao thread alive"; 
}
