#pragma once

#include <unordered_map>

#include <bodycontrol/internals/submodule.h>
#include <framework/joints/joints.hpp>
#include <framework/filter/exponential_moving_average.h>

/**
 * Avoids excessive heating of joints while bot is standing.
 * Mostly usefule for low stand, but runs all the time, kicking in
 * when legs have not been moved
 */

class EnergySaver : public SubModule {
    // tuning section
    static constexpr float delta{0.0013962634f};   ///< minimal joint step, value plugged from BerlinUnited code
    static inline const Angle maxOffset{1.0_deg};  ///< max. allowed offset (resets if exceeded)
    static constexpr float currentThreshold{0.3f}; ///< don't adjust offset if current is below this value
    static constexpr float motionThreshold{0.01};  ///< assume joint is moving if actuator difference exceeds this value
    static constexpr int checkInterval{1000};      ///< interval in milliseconds for offset updates
    // end tuning section

    std::map<JointNames, EMA> currentFiltered;
    joints::pos::Legs prevAct, offsets;
    TimestampMs lastCheck{0};
    bool reset{false};

public:
    EnergySaver() {
        offsets.fill(0);
        offsets.each([&](JointNames i) { currentFiltered.emplace(i, EMA(0, 0.85f)); });
    }

    SubModuleReturnValue step(BodyBlackboard *bb) override {
        auto current{joints::current::Legs(bb->sensors)};
        auto pos{joints::pos::Legs(bb->sensors)};
        joints::pos::Legs act(bb->actuators);

        // low-pass filter joint current sensors
        current.each([&](JointNames i) { currentFiltered.at(i) += current[i]; });

        // check if legs are moving or bot has been lifted
        if (!bb->qns[IS_STANDING] || !bb->qns[HAS_GROUND_CONTACT] ||
                !joints::details::feq(act, prevAct, motionThreshold)) {
            // reset offsets & abort
            if (!reset) {
                offsets.fill(0);
                reset = true;
            }
            prevAct = act;
            lastCheck = bb->timestamp_ms;
            return RUNNING;
        }

        // apply offsets
        act += offsets;
        act.write(bb->actuators);

        // only adjust offsets every checkInterval milliseconds
        if ((bb->timestamp_ms - lastCheck) < checkInterval)
            return RUNNING;

        // find index of motor with highest current
        JointNames max{JointNames::MIN};
        for (auto &[id, value] : currentFiltered) {
            if (max == JointNames::MIN || value > currentFiltered.at(max))
                max = id;
        }

        if (currentFiltered.at(max) > currentThreshold) {
            // adjust offset
            if (max == JointNames::LKneePitch || max == JointNames::RKneePitch)
                offsets[max] += delta;
            else
                offsets[max] -= delta;

            if (offsets[max] > maxOffset.rad()) {
                // this hopefully never happens
                offsets[max] = 0;
                LOG_WARN << "joint offset " << max << " exceeded threshold, resetting";
            }
        }

        reset = false;
        prevAct = act;
        lastCheck = bb->timestamp_ms;
        return RUNNING;
    }
};

// vim: set ts=4 sw=4 sts=4 expandtab:
