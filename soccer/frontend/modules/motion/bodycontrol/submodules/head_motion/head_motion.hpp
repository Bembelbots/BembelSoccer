#pragma once

#include <bodycontrol/internals/submodule.h>
#include <representations/bembelbots/constants.h>
#include <representations/motion/body_commands.h>
#include <framework/math/utils.h>

#include <optional>

class HeadMotion : public SubModule {

public:
    void setup(Setup s) override {
        s.cmds->connect<SetHeadMotion, &HeadMotion::setHeadMotion>(this);
        s.cmds->connect<SetHeadLookRCS, &HeadMotion::setHeadLookRCS>(this);
    }

    float easing_sin(float progress) {
        return 0.5f * std::sin(progress - M_PI_2_F) + 0.5f; 
    }

    float calc_pitch(float yaw_pos) {
        float sweep_progress = (std::abs(yaw_pos) - min_yaw_pitch_sweep) / (max_yaw_pitch_sweep - min_yaw_pitch_sweep);

        if (std::abs(yaw_pos) >= min_yaw_pitch_sweep ) { 
            return CONST::initial_pitch - pitch_sweep_range * easing_sin(sweep_progress * M_PI_F);
        } else {
            return CONST::initial_pitch;
        }
    }

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        float yaw_pos = bb->headYawLastPos;
        float pitch_pos = bb->headPitchLastPos;

        if (bb->qns[IS_FALLING] || bb->qns[IS_FALLEN] || bb->qns[IS_STANDING_UP])
            current_hm = HeadMotionType::NONE;

        switch(current_hm) {
            case HeadMotionType::SWEEP: {
                yaw_pos += hm_dir * yaw_sweep;

                if (std::abs(yaw_pos) >= max_yaw-0.05f) {
                    hm_dir *= -1;
                }

                pitch_pos = calc_pitch(yaw_pos);
            }
            break;
            case HeadMotionType::BALL:
            case HeadMotionType::FREE: {
                if(!look_at_angle.has_value()) {
                    look_at_angle = 0.f;
                }
                float yaw_diff = *look_at_angle - yaw_pos;
                //yaw_pos = 0.0f;
                yaw_pos += scale(yaw_diff, -max_yaw, max_yaw, -vmax_yaw, vmax_yaw);

                pitch_pos = calc_pitch(yaw_pos);
            }
            break;
            case HeadMotionType::NONE:
            default: {
                //bb->actuators[headPitchPositionActuator] = CONST::initial_pitch;
                yaw_pos -= scale(yaw_pos, -max_yaw, max_yaw, -yaw_sweep, yaw_sweep);
                
                pitch_pos = calc_pitch(yaw_pos);
            }
            break;
        }

        bb->actuators[headYawPositionActuator] = yaw_pos;
        bb->actuators[headPitchPositionActuator] = pitch_pos;
        
        bb->headYawLastPos = yaw_pos;
        bb->headPitchLastPos = pitch_pos;

        return RUNNING;
    }

    void setHeadMotion(SetHeadMotion hm)
    {
        current_hm = hm.value;
    }

    void setHeadLookRCS(SetHeadLookRCS pos)
    {
        look_at_angle = pos.value.angle().rad();
    }

private:
    HeadMotionType current_hm;
    std::optional<float> look_at_angle;
    float hm_dir = 1.f;

    // maximum value for yaw position (negative to positive)
    // (V6 can move up to 2.0857f)
    static constexpr float max_yaw = 1.5f;
    // maximum value for pitch position
    static constexpr float max_pitch = 0.2f;
    // minimum yaw where pitch leaves CONST::initial_pitch
    static constexpr float min_yaw_pitch_sweep = 0.1f;
    // yaw position where pitch reaches max_pitch
    static constexpr float max_yaw_pitch_sweep = M_PI_2_F;

    static constexpr float pitch_sweep_range = CONST::initial_pitch - max_pitch;

    // yaw speed for head sweeping motion
    static constexpr float yaw_sweep = 0.015f; //original 0.04f
    // maximum velocity for yaw
    static constexpr float vmax_yaw = 0.05f;
};
