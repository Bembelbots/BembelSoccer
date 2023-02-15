#pragma once

#include <vector>

#include <framework/math/directed_coord.h>
#include <framework/util/clock.h>
#include "../bembelbots/types.h"
#include "../bembelbots/constants.h"

/**
 * basis class of all objects sent around between modules, contains messageType, sender id, timestamp.
*/
class Message {
public:
    /// default ctor, setting sender id of robot
    Message();

    TimestampMs age() const;
    TimestampMs timestamp; ///< when did this happen (see note on \ref timestamps)
};


class Robot;

/** ball position, see Message for details. */
class  Ball: public Message {
public:
    Ball();
    Ball(const Coord &, float confidence=1.f);

    int id = 0;

    Coord pos;           ///< position (WCS)
    float posConfidence = 0.f; ///< confidence (see note on \ref uncertainty)

    Coord motion;           ///< motion
    float motionConfidence = 0.f; ///< confidence on motion (see note on \ref uncertainty)

    float localConfidence = 0.f; ///< added due to problems with robot-ranking. PLEASE, note to which data this confidence corresponds.

    Robot* robot = nullptr;

    void setPos(const Coord &p, const float &c = 1.0f);
    void setPos(const float &x, const float &y, const float &c = 1.0f);
    Ball wcs2rcs(const Robot &robotWcs) const;    ///< helper to convert from wcs to rcs
    Ball rcs2wcs(const Robot &robotWcs) const;    ///< helper to convert from rcs to wcs
};

/** robot position, see Message for details. */
class Robot: public Message {
public:
    /**
     * ctor for robot. Sets all fields to default values.
     */
    Robot();
    Robot(Coord pos);
    Robot(DirectedCoord pos);
    Robot(DirectedCoord pos, DirectedCoord GTpos);

    void setUnknownPose(); ///< sets all confidence and pose data to initial values

    // set robot from string
    bool setFromString(const std::string &data);

    int id; ///< id of the robot that has this pose (0..2 for our bots, else unknown)
    RobotRole role; ///< instance of robot role
    int fallen; ///< how long is the robot fallen down in seconds, -1 if not fallen.
    bool active; ///< is robot active, i.e. not penalized and so on

    DirectedCoord pos; ///< pos, see \ref coordsys
    float confidence; ///< confidence (see note on \ref confidence)

    DirectedCoord GTpos; ///< ground thruth position
    float GTconfidence; ///< ground thruth confidence
    TimestampMs GTtimestamp; ///< A Timestamp, @see getTimestampInMs()

    Ball *ball = nullptr;
};


std::ostream &operator<<(std::ostream &s,
                         const Ball &b); ///< stream output function
std::ostream &operator<<(std::ostream &s,
                         const Robot &r); ///< stream output function

using robotArray = std::array<Robot, NUM_PLAYERS>; // this avoids breaking blackboard macros
using ballArray  = std::array<Ball, NUM_PLAYERS>;  // dito
// vim: set ts=4 sw=4 sts=4 expandtab:
