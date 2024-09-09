#include <cmath>

#include "actuatorcheck.h"
#include "lola_names_generated.h"
#include <framework/joints/body_v6.h>
#include <framework/joints/joints.hpp>

// don't generate warning if limits are exceeded by less than this value
static constexpr float THRESHOLD{1e-10f};

bool checkAndCorrectActuators(bbipc::Actuators *actuators, const bbipc::Sensors &sensors) {
    constexpr auto &limits{joints::CONSTRAINTS};
    joints::pos::All pos(sensors);
    joints::pos::All joints(actuators);
    const auto errors{joints.isInvalid()};
    bool return_err{false};

    if (!errors)
        return true;

    for (const auto &i : *errors) {
        float v = joints[i];
        joints[i] = std::clamp(joints[i], limits.at(i).min, limits.at(i).max);
        if (std::abs(v - joints[i]) > THRESHOLD)
            return_err = true;
    }

    joints.write(actuators);
    return return_err;
}

