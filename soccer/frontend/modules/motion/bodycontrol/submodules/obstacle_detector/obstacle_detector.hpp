#pragma once
#include <cmath>
#include <bodycontrol/internals/submodule.h>
#include <framework/joints/joints.hpp>
#include <bodycontrol/utils/special_stances.hpp>
#include <framework/logger/logger.h>
#include <framework/math/angle.h>
#include <bodycontrol/utils/state_machine.h>
#include <numeric>

class ObstacleDetector : public SubModule {
private:
    joints::Stiffness<joints::Mask::LShoulderPitch | joints::Mask::RShoulderPitch> arms_stiffness;

    std::vector<float> l_shoulder_positions;
    std::vector<float> r_shoulder_positions;
    size_t l_frame_count;
    size_t r_frame_count;

    static constexpr float stiffness_low = 0.07f;
    static constexpr float stiffness_high = 0.3f;

    static constexpr int window_size = 20;
    static inline const Angle obstacle_delta = 5.0_deg;
    static inline const Angle max_update_offset = 1.0_deg;

    float shoulder_pitch_walking;

    SubModuleReturnValue BC_STATE_MACHINE(BodyBlackboard *bb) {
        BC_INITIAL_STATE(reset, RUNNING) {
            if(bb->prevQns[IS_WALKING]){
                goto init;
            }
        }

        BC_STATE(init, RUNNING) {
            BC_STATE_INITIALIZATION {
                l_frame_count = 0;
                r_frame_count = 0;
            }

            l_shoulder_positions[l_frame_count] = bb->sensors[lShoulderPitchPositionSensor];
            r_shoulder_positions[r_frame_count] = bb->sensors[rShoulderPitchPositionSensor];

            l_frame_count++;
            r_frame_count++;

            if(l_frame_count == window_size && r_frame_count == window_size){
                goto detect;
            }
        }

        BC_STATE(detect, RUNNING) {
            if(not bb->prevQns[IS_WALKING]){
                goto reset;
            }

            float lShoulderPitch = bb->sensors[lShoulderPitchPositionSensor];
            float rShoulderPitch = bb->sensors[rShoulderPitchPositionSensor];

            auto l_pitch_reference = calculateAverage(l_shoulder_positions);
            auto r_pitch_reference = calculateAverage(r_shoulder_positions);

            auto l_diff = abs(lShoulderPitch - l_pitch_reference);
            auto r_diff = abs(rShoulderPitch - r_pitch_reference);

            if (l_diff > obstacle_delta.rad()) {
                bb->qns[IS_OBSTACLE_LEFT] = true;
                arms_stiffness.fill(stiffness_high);
            }

            if (r_diff > obstacle_delta.rad()) {
                bb->qns[IS_OBSTACLE_RIGHT] = true;
                arms_stiffness.fill(stiffness_high);
            }

            if (l_diff <= max_update_offset.rad()) {
                l_shoulder_positions[l_frame_count % window_size] = lShoulderPitch;
                l_frame_count++;
            }

            if (r_diff <= max_update_offset.rad()) {
                r_shoulder_positions[r_frame_count % window_size] = rShoulderPitch;
                r_frame_count++;
            }

            if(!(bb->qns[IS_OBSTACLE_RIGHT] or bb->qns[IS_OBSTACLE_LEFT])){
                arms_stiffness.fill(stiffness_low);
            }

            arms_stiffness.write(bb->actuators);
        }

    }

    float calculateAverage(const std::vector<float> &measurements){
        assert(measurements.size() != 0);
        auto sum = std::accumulate(measurements.begin(), measurements.end(), 0.0f);
        return sum / measurements.size(); 
    }

public:
    ObstacleDetector()
    : l_shoulder_positions(window_size, 0.0f), 
        r_shoulder_positions(window_size, 0.0f) {}

    SubModuleReturnValue step(BodyBlackboard *bb) {
        return BC_RUN_STATE_MACHINE(bb);
    }

    void reset(){
        reset_state_machine();
    }
};
