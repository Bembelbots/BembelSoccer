#pragma once

#include "../../../walk/htwk/ankle_balancer.h"
#include "../../../kinematics/foot.h"
#include "../../../kinematics/step.h"
//#include "fast_math.h"
#include <framework/joints/joints.hpp>

#include <bodycontrol/blackboards/bodyblackboard.h>

#define NUM_SPLINE_POINTS 5

class MotionDesignBlackboard;

class MotionDesignEngine {

public:
    MotionDesignEngine();
    ~MotionDesignEngine();
    void proceed(BodyBlackboard *bb, MotionDesignBlackboard *calib, Step *step, const AnkleBalancer &ankle_balancer, float &stiffnesVal);

    void set_joints(Step *step, const AnkleBalancer &ankle_balancer, bool isInterpolating);

    static void set_stiffness(Step *step, MotionDesignBlackboard* calib);

    void write_to_blackboard(MotionDesignBlackboard* calib);

    void reset();

private:

    float interpolate_joint(float current, float target, float percentage);

    uint32_t start_time = 0;
    float interpolationPercentage;
    bool isNewFrame = false;
    joints::pos::Old start;
    joints::pos::Old current;
    joints::pos::Old target;
    
};
