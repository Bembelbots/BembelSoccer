#pragma once

#include "motion.h"
#include "../bembelbots/types.h"
#include "../camera/cam_pose_struct.h"
#include <framework/rt/message_utils.h>
#include <framework/math/directed_coord.h>
#include <framework/util/enum/serializable_enum.h>
#include <framework/joints/joints.hpp>

#include <Eigen/Core>

#include <bitset>
#include <array>
#include <vector>

// clang-format off
SERIALIZABLE_ENUM(BodyQuestion,
    (IS_STANDING) (0),
    (IS_STANDING_UP),
    (IS_FALLEN),
    (IS_FALLING),
    
    (IS_WALKING),

    (RAISING_LEFT_ARM_DONE),
    
    (HAS_GROUND_CONTACT),
    
    (CHEST_BUTTON_PRESSED),
    (CHEST_BUTTON_DOUBLE_CLICK),
    (CHEST_BUTTON_TRIPPLE_CLICK),
    (CHEST_BUTTON_DOWN),
    (CHEST_BUTTON_UP),
    
    (LEFT_BUMPER_PRESSED),
    (LEFT_BUMPER_DOUBLE_CLICK),
    (LEFT_BUMPER_TRIPPLE_CLICK),
    (LEFT_BUMPER_DOWN),
    (LEFT_BUMPER_UP),
    
    (RIGHT_BUMPER_PRESSED),
    (RIGHT_BUMPER_DOUBLE_CLICK),
    (RIGHT_BUMPER_TRIPPLE_CLICK),
    (RIGHT_BUMPER_DOWN),
    (RIGHT_BUMPER_UP),
    
    (FRONT_HEAD_TOUCH_PRESSED),
    (FRONT_HEAD_TOUCH_DOUBLE_CLICK),
    (FRONT_HEAD_TOUCH_TRIPPLE_CLICK),
    (FRONT_HEAD_TOUCH_DOWN),
    (FRONT_HEAD_TOUCH_UP),
    
    (MIDDLE_HEAD_TOUCH_PRESSED),
    (MIDDLE_HEAD_TOUCH_DOUBLE_CLICK),
    (MIDDLE_HEAD_TOUCH_TRIPPLE_CLICK),
    (MIDDLE_HEAD_TOUCH_DOWN),
    (MIDDLE_HEAD_TOUCH_UP),
    
    (REAR_HEAD_TOUCH_PRESSED),
    (REAR_HEAD_TOUCH_DOUBLE_CLICK),
    (REAR_HEAD_TOUCH_TRIPPLE_CLICK),
    (REAR_HEAD_TOUCH_DOWN),
    (REAR_HEAD_TOUCH_UP),
    
    (IS_OBSTACLE_LEFT),
    (IS_OBSTACLE_RIGHT),
    
    (NUM_OF_BODY_QUESTIONS) // Must be last
);
// clang-format on

// TODO Rename to BodyMessage for consistency
class BodyState {

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    uint32_t timestamp_ms;
    uint32_t tick;
    int64_t lola_timestamp; //< recv. timestamp from lola backend, used to match camera images

    camPose bCamPose, tCamPose;

    Eigen::Vector3f accel;
    Eigen::Vector3f gyro;
    Eigen::Vector3f bodyAngles;

    DirectedCoord odometry;

    joints::Sensors sensors; //< raw sensor values

    FallenSide fallenSide = FallenSide::NONE;

    Motion activeMotion;

    std::bitset<NUM_OF_BODY_QUESTIONS> qns;

    float lastHeadYaw = 0.0f;
    float lastHeadPitch = 0.0f;

    std::array<Eigen::Vector3f, 6> com;
};

template<>
struct rt::Squash<BodyState> {
    void operator()(BodyState &base, const BodyState &newer) {

        auto old = std::exchange(base, newer);

        static const std::vector<BodyQuestion> orQns = {
                CHEST_BUTTON_PRESSED,
                CHEST_BUTTON_DOUBLE_CLICK,
                CHEST_BUTTON_TRIPPLE_CLICK,

                LEFT_BUMPER_PRESSED,
                LEFT_BUMPER_DOUBLE_CLICK,
                LEFT_BUMPER_TRIPPLE_CLICK,

                RIGHT_BUMPER_PRESSED,
                RIGHT_BUMPER_DOUBLE_CLICK,
                RIGHT_BUMPER_TRIPPLE_CLICK,
             
                FRONT_HEAD_TOUCH_PRESSED,
                FRONT_HEAD_TOUCH_DOUBLE_CLICK,
                FRONT_HEAD_TOUCH_TRIPPLE_CLICK,
                
                MIDDLE_HEAD_TOUCH_PRESSED,
                MIDDLE_HEAD_TOUCH_DOUBLE_CLICK,
                MIDDLE_HEAD_TOUCH_TRIPPLE_CLICK,
                
                REAR_HEAD_TOUCH_PRESSED,
                REAR_HEAD_TOUCH_DOUBLE_CLICK,
                REAR_HEAD_TOUCH_TRIPPLE_CLICK,
        };

        for (auto question : orQns) {
            base.qns[question] = base.qns[question] || old.qns[question];
        }
    }
};
