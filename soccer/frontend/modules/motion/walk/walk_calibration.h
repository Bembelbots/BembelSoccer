#pragma once

#include <framework/blackboard/blackboard.h>

class ConfigParser;


class WalkCalibration : public Blackboard {
public:

    static const std::string blackboardName;

    explicit WalkCalibration();
    ~WalkCalibration();

    bool loadConfig(ConfigFiles &) override;
    bool writeConfig(ConfigFiles &) override;

    MAKE_VAR(float, step_duration); // s
    MAKE_VAR(float, max_forward);  // m/s
    MAKE_VAR(float, max_backward);  // m/s
    MAKE_VAR(float, max_strafe);  // m/s
    MAKE_VAR(float, max_turn);  // rad/s
    MAKE_VAR(float, max_acceleration_forward);  // m/s/s
    MAKE_VAR(float, max_deceleration_forward);  // m/s/s
    MAKE_VAR(float, max_acceleration_side);  // m/s/s
    MAKE_VAR(float, max_acceleration_turn);  // rad/s/s
    MAKE_VAR(float, min_rel_step_duration);
    MAKE_VAR(float, max_rel_step_duration);
    MAKE_VAR(float, step_duration_factor);
    MAKE_VAR(float, feet_angle);
    MAKE_VAR(float, body_height);
    MAKE_VAR(float, body_height_stand);
    MAKE_VAR(float, body_shift_amp);
    MAKE_VAR(float, body_shift_smoothing);
    MAKE_VAR(float, body_offset_x);
    MAKE_VAR(float, waddle_gain);
    MAKE_VAR(float, stairway_gain);
    MAKE_VAR(float, basic_step_height);
    MAKE_VAR(float, forward_step_height);
    MAKE_VAR(float, support_recover_factor);
    MAKE_VAR(float, velocity_combination_damping);
    
    MAKE_VAR(float, hip_angle);
    MAKE_VAR(float, elbow_yaw);
    MAKE_VAR(float, elbow_roll);
    MAKE_VAR(float, shoulder_pitch);
    MAKE_VAR(float, shoulder_roll);

    MAKE_VAR(float, scale_odo_x);
    MAKE_VAR(float, scale_odo_y);

private:

    static const std::string cfgSection;

};

std::ostream &operator<<(std::ostream &s, const WalkCalibration &rhs);

// vim: set ts=4 sw=4 sts=4 expandtab:

