#pragma once
#include <tuple>
#include <vector>

#include <initializer_list>
#include <iterator>
#include <framework/joints/definitionsBody.h>
#include <framework/joints/joints.hpp>
#include <framework/logger/logger.h>

struct StiffnessTag {};

struct KeyframeJoint {
    const joint_id jointId;
    const float position;
    const float stiffnessStart;
    const float stiffnessEnd;
    const bool  stiffnessOnly;

    KeyframeJoint(joint_id jointId, float position, float start_stiffness = -2.0f, float end_stiffness = -2.0f)
        :   jointId(jointId), position(position),
            stiffnessStart(start_stiffness), stiffnessEnd(end_stiffness), stiffnessOnly(false) {
    } 
    
    KeyframeJoint(joint_id jointId, StiffnessTag tag, float start_stiffness, float end_stiffness = -2.0f)
        :   jointId(jointId), position(0.0f),
            stiffnessStart(start_stiffness), stiffnessEnd(end_stiffness), stiffnessOnly(true) {
    } 
};

class Keyframe {
public:
    using Stance = std::vector<KeyframeJoint>;
    using stance_pos_type = joints::pos::All;
    using stance_stiffness_type = joints::stiffness::All;
    using stance_tuple_type = std::tuple<stance_pos_type, stance_stiffness_type, stance_stiffness_type>;
    
    using motion_type = joints::Linear<stance_pos_type>;
    using storage_type = std::vector<KeyframeJoint>;
    using return_type = std::tuple<stance_pos_type, stance_stiffness_type>;

    Keyframe(Stance positions, int duration, float stiffness_start = -2.0f, float stiffness_end = -2.0f)
        : positions(positions), duration(duration), stiffnessStartDefault(stiffness_start), stiffnessEndDefault(stiffness_end) {
        this->duration = duration;

        if(stiffnessStartDefault > -2.0f && stiffnessEndDefault <= -2.0f) {
            stiffnessEndDefault = stiffnessStartDefault;
        }
        reset();
    }


    stance_tuple_type calculateStance(const stance_pos_type &current_stance, const stance_stiffness_type &current_stiffness) const {
        stance_pos_type stance = current_stance;
        stance_stiffness_type stiffness_start = current_stiffness;
        stance_stiffness_type stiffness_end = current_stiffness;

        for(auto &jointPos : positions) {
            if(not jointPos.stiffnessOnly) {
                stance[jointPos.jointId] = jointPos.position;
            }

            float stiffnessStart = (jointPos.stiffnessStart > -2.0f ? jointPos.stiffnessStart : stiffnessStartDefault);
            float stiffnessEnd = (jointPos.stiffnessEnd > -2.0f ? jointPos.stiffnessEnd : stiffnessEndDefault);

            if(stiffnessStart > -2.0f) {
                stiffness_start[jointPos.jointId] = stiffnessStart;
            }

            if(stiffnessEnd > -2.0f) {
                stiffness_end[jointPos.jointId] = stiffnessEnd;
            }
        }

        return { stance, stiffness_start, stiffness_end };
    }

    return_type run(const joints::Sensors &sensors, const joints::Actuators &actuators, int timestamp) {
        if(isDone) {
            return getCurrentValues(sensors, actuators);
        }
        
        if(wasReset) {
            wasReset = false;
            auto [current_stance, current_stiffness] = getCurrentValues(sensors, actuators);
            auto [stance, stiffnessStart, stiffnessEnd] = calculateStance(current_stance, current_stiffness);
            
            motion.setDuration(duration);
            motion.setStart(timestamp);
            motion.setFrom(current_stance);
            motion.setTo(stance);

            this->stiffnessStart = stiffnessStart;
            this->stiffnessEnd = stiffnessEnd;
        }

        auto ret = motion.get(timestamp);
        isDone = motion.done(timestamp);

        if(isDone) {
            return {ret, this->stiffnessEnd };
        }

        return {ret, this->stiffnessStart };
    }

    bool done() {
        return isDone;
    }

    void reset() {
        isDone = false;
        wasReset = true;
    }

private:
    storage_type positions;
    int duration;
    float stiffnessStartDefault;
    float stiffnessEndDefault;
    
    bool isDone   { false };
    bool wasReset { false };

    motion_type motion;
    stance_stiffness_type stiffnessStart;
    stance_stiffness_type stiffnessEnd;

    return_type getCurrentValues(const joints::Sensors &sensors, const joints::Actuators &actuators) {
        auto[current_stance, current_stiffness] = return_type{};
        current_stance.read(sensors);
        current_stiffness.read(actuators);
        return {current_stance, current_stiffness};
    }
};

class Movement {
public: 
    using storage_type = std::vector<Keyframe>;
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    Movement(std::initializer_list<Keyframe> keyframes)
        : keyframes(keyframes) {
        reset();
    }

    bool run(const joints::Sensors &sensors, joints::Actuators &actuators, int timestamp) {
        if(done()) {
            return false;
        }
        
        auto &keyframe = *current;
        if(not keyframe.done()) {
            auto [stance, stiffness] = keyframe.run(sensors, actuators, timestamp);
            stance.write(actuators);
            stiffness.write(actuators);
        } else {
            advanceKeyframe();
        }

        bool isDone = done();
        if(isDone) {
            // relax joints by accepting current position as target
            joints::pos::All current;
            current.read(sensors);
            current.write(actuators);
        }
        return not isDone;
    }

    bool done() {
        return current == std::end(keyframes);
    }

    void reset() {
        advanceKeyframe(true);
    }

    const_iterator begin() const {
        return keyframes.begin();
    } 
    
    const_iterator end() const {
        return keyframes.end();
    } 

private:

    storage_type keyframes;
    iterator current;

    void advanceKeyframe(bool wasReset = false) {
        if(not wasReset && done()) {
            return;
        }

        if(wasReset) {
            current = std::begin(keyframes);
        } else {
            std::advance(current, 1);
        }
        
        (*current).reset();
    }
};
