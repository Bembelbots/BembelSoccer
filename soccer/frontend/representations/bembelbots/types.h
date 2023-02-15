#pragma once

#include <framework/util/enum-flags/flags.hpp>
#include <framework/util/enum/serializable_enum.h>
#include <cassert>
#include <vector>

// type to keep unix stamp in microseconds
typedef int64_t microTime;

// type to keep unix stamp in milliseconds
typedef int32_t TimestampMs;

// Coordsystem bases
enum class CoordSys {
    WCS,
    RCS,
    MCS,
    ICS
};

enum class StabilizationType {
    DEACTIVATED = 0,
    ARMS = 1
};

// Possible field sizes
SERIALIZABLE_ENUM_CLASS(FieldSize,
    (JRL) (0),
    (SPL) (1),
    (HTWK) (2),
    (TINY) (3),
    (JSON) (255)
);


// Robot role in game
SERIALIZABLE_ENUM_CLASS(RobotRole,
    (NONE) (0),
    (STRIKER),
    (DEFENDER),
    (GOALKEEPER),
    (SUPPORTER_DEFENSE),
    (SUPPORTER_OFFENSE),
    (SEARCHER),
    (DEMO),
    (PENALTYKICKER),
    (PENALTYGOALIE),
    (OBSTACLE_AVOIDER),
    (PASSING_ROBOT)
);


// Enum for BotId that are named after jersey numbers but keep the internal
// numbering
enum class BotId {
    One = 0,
    Two = 1,
    Three = 2,
    Four = 3,
    Five = 4,
};

[[maybe_unused]] inline constexpr int operator*(BotId id) {
    return static_cast<int>(id);
}

[[maybe_unused]] inline constexpr BotId fromJersey(int jersey) {
    assert(1 <= jersey && jersey <= 5);
    return BotId(jersey-1);
}

[[maybe_unused]] inline constexpr int toJersey(BotId id) {
    return *id + 1;
}

// On which side does the robot lie?
SERIALIZABLE_ENUM_CLASS(FallenSide,
    (NONE),
    (FRONT),
    (BACK)
);

enum class ImageCodec: uint8_t {
    JPG = 1,
    PNG = 0,
};


// All available arm positions
enum class ArmsPosition {
    NONE,
    BACK,
    FRONT,
    SIDE,
    SAFE
};


// Head motion type (what to pay attention to?)
enum class HeadMotionType {
    //NONE: stop head motion and go to initial positions
    NONE,
    //BALL: head track, means tracking of object: look to ball or sweep head to find ball
    BALL,
    // FREE: ?? track objekt: landmark or ball
    FREE,
    //head sweep, means move the head left-to-right
    SWEEP
};


// RGB - colors
enum class RGBColor : uint32_t {
    BLACK = 0x000000,
    OFF = BLACK,
    WHITE = 0xFFFFFF,
    GRAY = 0xC0C0C0,
    RED = 0xFF0000,
    BLUE = 0x0000FF,
    YELLOW = 0xFFD700,
    GREEN = 0x00FF00,
    DARKISH_GREEN = 0x008000,
    PINK = 0xFF1493,
    BROWN = 0xD2691E,
    PURPLE = 0x800080,
};

// RGB - colors
enum class RGBAColor : uint32_t {
    //AARRGGBB
    WHITE_TRANSP = 0xAAFFFFFF,
    BLACK_TRANSP = 0xAA000000,
    GRAY_TRANSP = 0xAAC0C0C0
};


SERIALIZABLE_ENUM_CLASS(LED,
    (CHEST),
    (BRAIN),
    (BRAIN_RIGHT),
    (BRAIN_LEFT),
    (RIGHT_EYE),
    (RIGHT_EYE_LEFT),
    (RIGHT_EYE_RIGHT),
    (LEFT_EYE),
    (LEFT_EYE_LEFT),
    (LEFT_EYE_RIGHT),
    (RIGHT_EAR),
    (LEFT_EAR),
    (RIGHT_FOOT),
    (LEFT_FOOT),
    (ALL)
);


// Rough areas on the field, meant as position or target hints
enum class FieldArea {
    OWN_HALF = 0,
    OPPONENT_HALF,
    OWN_HALF_LEFT_QUADRANT,
    OWN_HALF_RIGHT_QUADRANT,
    OPPONENT_HALF_LEFT_QUADRANT,
    OPPONENT_HALF_RIGHT_QUADRANT,
    CENTER_CIRCLE,
    OWN_PENALTY_BOX,
    ENEMY_PENALTY_BOX,
    MAX
};


enum class StiffnessCommand {
    UNSTIFF_ALL,
    RESTIFF_ALL,
    FORCE_STIFF,
    FORCE_UNSTIFF,
    NONE,
};

enum class BodyHeight {
    NORMAL,
    HIGH,
};

// landmarks by their type (without 'side' information)
enum class LandmarkType {
    CENTER_CIRCLE,
    PENALTY_POINT,
    PENALTY_BOX,
    GOAL,
    GOAL_POLE,
    CORNER,
    LINE_T,
    LINE_X,
    LINE_L,
    LINE
};

// corresponds to software/bembelDbug/config/default.json, change values there too
enum class RemoteMotions {
    STAND = 0,
    TIPPLE = 1,
    WALK_FORWARD = 2,
    WALK_BACKWARD =3,
    TURN_L = 4,
    TURN_R = 5,
    STRAFE_L = 6,
    STRAFE_R = 7,
    KICK=8,
    STAND_UP=9,
    RAW=10,
    DESIGN=11,
};


// vim: set ts=4 sw=4 sts=4 expandtab:
