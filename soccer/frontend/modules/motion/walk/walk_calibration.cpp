#include "walk_calibration.h"

#include <framework/util/configparser.h>
#include <framework/logger/logger.h>

#include <iomanip>
#include <ostream>


const std::string WalkCalibration::blackboardName = "WalkCalibration";

// keep the old name, so we dont have to touch the config on every robot...
const std::string WalkCalibration::cfgSection = "motion"; 


WalkCalibration::WalkCalibration()
    : Blackboard(blackboardName) {

    INIT_VAR_RW(step_duration, 0.27f, ""); // s
    INIT_VAR_RW(max_forward, 0.2f, "");  // m/s
    INIT_VAR_RW(max_backward, 0.15f, "");  // m/s
    INIT_VAR_RW(max_strafe, 0.35f, "");  // m/s
    INIT_VAR_RW(max_turn, 1.0f, "");  // rad/s
    INIT_VAR_RW(max_acceleration_forward, 0.08f, "");  // m/s/s
    INIT_VAR_RW(max_deceleration_forward, 0.12f, "");  // m/s/s
    INIT_VAR_RW(max_acceleration_side, 0.12f, "");  // m/s/s
    INIT_VAR_RW(max_acceleration_turn, 1.2f, "");  // rad/s/s
    INIT_VAR_RW(min_rel_step_duration, 0.75f, "");
    INIT_VAR_RW(max_rel_step_duration, 3.0f, "");
    INIT_VAR_RW(step_duration_factor, 1.0f, "");
    INIT_VAR_RW(feet_angle, 0.0f, "");
    INIT_VAR_RW(body_height, 0.19f, "");
    INIT_VAR_RW(body_height_stand, 0.203f, "");
    INIT_VAR_RW(body_shift_amp, -0.08f, "");
    INIT_VAR_RW(body_shift_smoothing, 0.95f, "");
    INIT_VAR_RW(body_offset_x, -0.01f, "");
    INIT_VAR_RW(waddle_gain, 0.0089f, "");
    INIT_VAR_RW(stairway_gain,  0.0093f, "");
    INIT_VAR_RW(basic_step_height, 0.0135f, "");
    INIT_VAR_RW(forward_step_height, 0.0142f, "");
    INIT_VAR_RW(support_recover_factor, 0.3334f, "");
    INIT_VAR_RW(velocity_combination_damping, 0.5f, "");

    INIT_VAR_RW(hip_angle, 0.f, "");
    INIT_VAR_RW(elbow_yaw, -1.6f, "");
    INIT_VAR_RW(elbow_roll, -0.3f, "");
    INIT_VAR_RW(shoulder_pitch, 1.75f, "");
    INIT_VAR_RW(shoulder_roll, 0.15f, "");

    INIT_VAR_RW(scale_odo_x, 1.0f,"");
    INIT_VAR_RW(scale_odo_y, 1.0f,"");
}

WalkCalibration::~WalkCalibration() {}

bool WalkCalibration::writeConfig(ConfigFiles &fcfg) {
    auto *cfg = fcfg.calib;

    WRITE_KEY_SUBSECTION(cfg, cfgSection, step_duration, float); // s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_forward, float);  // m/s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_backward, float);  // m/s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_strafe, float);  // m/s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_turn, float);  // rad/s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_acceleration_forward, float);  // m/s/s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_deceleration_forward, float);  // m/s/s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_acceleration_side, float);  // m/s/s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_acceleration_turn, float);  // rad/s/s
    WRITE_KEY_SUBSECTION(cfg, cfgSection, min_rel_step_duration, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, max_rel_step_duration, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, step_duration_factor, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, feet_angle, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, body_height, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, body_height_stand, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, body_shift_amp, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, body_shift_smoothing, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, body_offset_x, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, waddle_gain, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, stairway_gain, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, basic_step_height, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, forward_step_height, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, support_recover_factor, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, velocity_combination_damping, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, scale_odo_x, float);
    WRITE_KEY_SUBSECTION(cfg, cfgSection, scale_odo_y, float);
    return true;
}

bool WalkCalibration::loadConfig(ConfigFiles &fcfg) {
    auto *cfg = fcfg.calib;

    try {
        READ_KEY_SUBSECTION(cfg, cfgSection, step_duration, float); // s
        READ_KEY_SUBSECTION(cfg, cfgSection, max_forward, float);  // m/s
        READ_KEY_SUBSECTION(cfg, cfgSection, max_backward, float);  // m/s
        READ_KEY_SUBSECTION(cfg, cfgSection, max_strafe, float);  // m/s
        READ_KEY_SUBSECTION(cfg, cfgSection, max_turn, float);  // rad/s
        READ_KEY_SUBSECTION(cfg, cfgSection, max_acceleration_forward, float);  // m/s/s
        READ_KEY_SUBSECTION(cfg, cfgSection, max_deceleration_forward, float);  // m/s/s
        READ_KEY_SUBSECTION(cfg, cfgSection, max_acceleration_side, float);  // m/s/s
        READ_KEY_SUBSECTION(cfg, cfgSection, max_acceleration_turn, float);  // rad/s/s
        READ_KEY_SUBSECTION(cfg, cfgSection, min_rel_step_duration, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, max_rel_step_duration, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, step_duration_factor, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, feet_angle, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, body_height, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, body_height_stand, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, body_shift_amp, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, body_shift_smoothing, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, body_offset_x, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, waddle_gain, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, stairway_gain, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, basic_step_height, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, forward_step_height, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, support_recover_factor, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, velocity_combination_damping, float);
        READ_KEY_SUBSECTION(cfg, cfgSection, scale_odo_x, float);    
        READ_KEY_SUBSECTION(cfg, cfgSection, scale_odo_y, float);
    } catch(std::runtime_error &e) {
        LOG_ERROR << e.what();
        return false;
    }

    LOG_INFO_FIRST_N(1) << "Walk calibration read from config file.";

    LOG_INFO << "WALK CALIBRATION";
    LOG_INFO << *this;

    return true;
}

std::ostream &operator<<(std::ostream &s, const WalkCalibration &rhs) {
    s << "current motion calibration blackboard content:\n";
    s << "  step_duration:                  " << std::setw(10) << rhs.step_duration << "\n";
    s << "  max_forward:                    " << std::setw(10) << rhs.max_forward << "\n";
    s << "  max_backward:                   " << std::setw(10) << rhs.max_backward << "\n";
    s << "  max_strafe:                     " << std::setw(10) << rhs.max_strafe << "\n";
    s << "  max_turn:                       " << std::setw(10) << rhs.max_turn << "\n";
    s << "  max_acceleration_forward:       " << std::setw(10) << rhs.max_acceleration_forward << "\n";
    s << "  max_deceleration_forward:       " << std::setw(10) << rhs.max_deceleration_forward << "\n";
    s << "  max_acceleration_side:          " << std::setw(10) << rhs.max_acceleration_side << "\n";
    s << "  max_acceleration_turn:          " << std::setw(10) << rhs.max_acceleration_turn << "\n";
    s << "  min_rel_step_duration:          " << std::setw(10) << rhs.min_rel_step_duration << "\n";
    s << "  max_rel_step_duration:          " << std::setw(10) << rhs.max_rel_step_duration << "\n";
    s << "  step_duration_factor:           " << std::setw(10) << rhs.step_duration_factor << "\n";
    s << "  feet_angle:                     " << std::setw(10) << rhs.feet_angle << "\n";
    s << "  body_height:                    " << std::setw(10) << rhs.body_height << "\n";
    s << "  body_height_stand:              " << std::setw(10) << rhs.body_height_stand<< "\n";
    s << "  body_shift_amp:                 " << std::setw(10) << rhs.body_shift_amp << "\n";
    s << "  body_shift_smoothing:           " << std::setw(10) << rhs.body_shift_smoothing << "\n";
    s << "  body_offset_x:                  " << std::setw(10) << rhs.body_offset_x << "\n";
    s << "  waddle_gain:                    " << std::setw(10) << rhs.waddle_gain << "\n";
    s << "  stairway_gain:                  " << std::setw(10) << rhs.stairway_gain << "\n";
    s << "  basic_step_height:              " << std::setw(10) << rhs.basic_step_height << "\n";
    s << "  forward_step_height:            " << std::setw(10) << rhs.forward_step_height << "\n";
    s << "  support_recover_factor:         " << std::setw(10) << rhs.support_recover_factor << "\n";
    s << "  velocity_combination_damping:   " << std::setw(10) << rhs.velocity_combination_damping << "\n";;
    s << "  odo_scale_x                     " << std::setw(10) << rhs.scale_odo_x << "\n";
    s << "  odo_scale_y                     " << std::setw(10) << rhs.scale_odo_y;

    return s;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
