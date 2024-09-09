#pragma once

#include "framework/joints/joints.hpp"
#include "lola_names_generated.h"
#include <array>

#include <representations/flatbuffers/types/actuators.h>
#include <representations/flatbuffers/types/sensors.h>

#include <framework/joints/special_stances.hpp>

static bool sit(const bbipc::Sensors &sensors, bbipc::Actuators &actuators) {
    static int time{0};
    static joints::Linear<joints::pos::All> motion;
    joints::pos::All jpos(sensors);
    joints::stiffness::All stiff;

    if (time == 0) {
        stiff.fill(0.6);
        stiff.write(actuators);
        
        motion.setStart(0);
        motion.setDuration(1000);

        motion.setFrom(jpos);

        motion.setTo(SIT_STANCE);
    }

    jpos = motion.get(time);
    jpos.write(actuators);

    if (motion.done(time)) {
        time = 0;

        stiff.fill(-1);
        stiff[JointNames::LHipPitch] = stiff[JointNames::RHipPitch] = 0.15f;
        stiff.write(actuators);
        
        actuators.joints.position = sensors.joints.position;
        
        return true;
    }

    time += 10;
    return false;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
