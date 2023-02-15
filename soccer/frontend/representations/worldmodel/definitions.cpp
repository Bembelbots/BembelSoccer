#include <cmath>
#include <cstring>

#include <string>
#include <iomanip>
#include <sstream>

#include <framework/util/assert.h>
#include "../bembelbots/constants.h"
#include "definitions.h"

/// confidence downweighting for walk (i.e. after walking ... m, conficence a of 1 will be 0)
static const float ROBOT_VARIANCE_X_FOR_CONF_ZERO = 3.0f / 3.0f;
/// confidence downweighting for walk (i.e. after walking ... m, conficence a of 1 will be 0)
static const float ROBOT_VARIANCE_Y_FOR_CONF_ZERO = 2.0f / 3.0f;
/// confidence downweighting for turn (i.e. after turning ... rad, conficence a of 1 will be 0)
static const float ROBOT_VARIANCE_TURN_FOR_CONF_ZERO = M_PI_F / 3.0f;

Ball::Ball() :
    Message(),
    id(-1),
    posConfidence(0),
    motionConfidence(0),
    localConfidence(0) {
}

Ball::Ball(const Coord &p, float c) :
    pos(p),
    posConfidence(c) {
}

void Ball::setPos(const Coord &p, const float &c) {
    pos = p;
    posConfidence = c;
    timestamp = getTimestampMs();
}

void Ball::setPos(const float &x, const float &y, const float &c) {
    setPos({x, y}, c);
}


Ball Ball::wcs2rcs(const Robot &robotWcs) const {
    // FIXME: Use conversion from directed coord!
    Ball ballRcs(*this);
    // transform from wcs to rcs using robot pose....
    // 1) POSE
    // 1a) add translation
    ballRcs.pos.sub(robotWcs.pos.coord);
    // 1b) rotate local coordinates according to alpha!
    float alpha = -robotWcs.pos.angle.rad();
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);
    float x      = ballRcs.pos.x * cosalpha - ballRcs.pos.y * sinalpha;
    ballRcs.pos.y = ballRcs.pos.x * sinalpha + ballRcs.pos.y * cosalpha;
    ballRcs.pos.x = x;
    // 1c) confidence
    ballRcs.posConfidence = posConfidence * robotWcs.confidence;

    // 2) MOTION
    // 2a) translation not needed!
    // 2b) rotate locat coordinates according to alpha
    ballRcs.motion.y = ballRcs.motion.x * sinalpha + ballRcs.motion.y * cosalpha;
    ballRcs.motion.x = x;
    // 2c) confidence
    ballRcs.motionConfidence = motionConfidence *
                               robotWcs.confidence; // this may not make so much sense, any suggestions?
    ballRcs.localConfidence = posConfidence;
    ballRcs.timestamp = timestamp;

    return ballRcs;
}

Ball Ball::rcs2wcs(const Robot &robotWcs) const {
    // FIXME: Use conversion from directed coord!
    Ball ballWcs(*this);
    // transform from rcs to wcs using robot pose....
    // 1) POSE
    // 1a) de-rotate local coordinates according to alpha!
    float alpha = robotWcs.pos.angle.rad();
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);
    float x      = ballWcs.pos.x * cosalpha - ballWcs.pos.y * sinalpha;
    ballWcs.pos.y = ballWcs.pos.x * sinalpha + ballWcs.pos.y * cosalpha;
    ballWcs.pos.x = x;
    // 1b) add translation
    ballWcs.pos.add(robotWcs.pos.coord);
    // 1c) confidence
    ballWcs.posConfidence = posConfidence * robotWcs.confidence;

    // 2) MOTIONS
    // 2a) de-rotate local coordinates according to alpha!
    x               = ballWcs.motion.x * cosalpha - ballWcs.motion.y * sinalpha;
    ballWcs.motion.y = ballWcs.motion.x * sinalpha + ballWcs.motion.y * cosalpha;
    ballWcs.motion.x = x;
    // 2b) translation not needed
    // 2c) confidence
    ballWcs.motionConfidence = motionConfidence *
                               robotWcs.confidence; // this may not make so much sense, any suggestions?
    ballWcs.localConfidence = posConfidence;
    ballWcs.timestamp = timestamp;

    return ballWcs;
}

Robot::Robot() :
    Message(),
    id(-1),
    role(RobotRole::STRIKER),
    fallen(-1),
    active(false),
    confidence(0),
    GTconfidence(0),
    GTtimestamp(-1) {
}

Robot::Robot(Coord pos) :
    Message(),
    id(-1),
    role(RobotRole::STRIKER),
    fallen(-1),
    active(false),
    confidence(0),
    GTconfidence(0),
    GTtimestamp(-1) {
    this->pos = DirectedCoord();
    this->pos.coord = pos;
}

Robot::Robot(DirectedCoord pos) :
    Message(),
    id(-1),
    role(RobotRole::STRIKER),
    fallen(-1),
    active(false),
    confidence(0),
    GTconfidence(0),
    GTtimestamp(-1) {
    this->pos = pos;
}
Robot::Robot(DirectedCoord pos, DirectedCoord GTpos) :
    Message(),
    id(-1),
    role(RobotRole::STRIKER),
    fallen(-1),
    active(false),
    confidence(0),
    GTconfidence(0),
    GTtimestamp(-1) {
    this->pos = pos;
    this->GTpos = GTpos;
}

void Robot::setUnknownPose() {
    pos = DirectedCoord();
    confidence = 0.0f;
}

bool Robot::setFromString(const std::string &data) {
    int pos1, pos2;

    pos1 = data.find("id=");
    id = atoi(data.substr(pos1+3, 1).c_str());

    pos1 = data.find("t=")+2;
    pos2 = data.find(" c=");
    timestamp = atoi(data.substr(pos1, pos2-pos1).c_str());

    pos1 = data.find(" c=")+3;
    pos2 = data.find(" @ ");
    confidence = atof(data.substr(pos1, pos2-pos1).c_str());

    pos1 = data.find(" @ ")+3;
    pos2 = data.find("rad ");
    std::string p = data.substr(pos1, pos2-pos1);
    pos1 = p.find(", ");
    pos.coord.x = atof(p.substr(0, pos1).c_str());
    pos2 = p.find(", ", pos1+2);
    pos.coord.y = atof(p.substr(pos1+2, pos2-pos1-2).c_str());
    pos.angle = Rad{static_cast<float>(atof(p.substr(pos2 + 2, p.size() - pos2 - 2).c_str()))};

    pos1 = data.find("RUTH: ")+6;
    pos2 = data.find(",Conf");
    std::string gt = data.substr(pos1, pos2-pos1);
    pos1 = gt.find(",");
    pos2 = gt.find(",", pos1+1);
    GTpos.coord.x = atof(gt.substr(0, pos1).c_str());
    GTpos.coord.y = atof(gt.substr(pos1+1, pos2-pos1-1).c_str());
    GTpos.angle = Rad{static_cast<float>(atof(gt.substr(pos2 + 1, p.size() - pos2 - 1).c_str()))};

    pos1 = data.find("Conf: ")+6;
    GTconfidence = atof(data.substr(pos1, data.size() - pos1).c_str());

    return true;
}

Message::Message() :
    timestamp(getTimestampMs()) {
}

TimestampMs Message::age() const {
    if (timestamp < 0) {
        return 1000*1000;    //getTimestampMs();
    }
    return getTimestampMs() - timestamp;
}

std::ostream &operator<<(std::ostream &s, const Ball &b) {
    s << "Ball id=" << b.id << " t=" << b.timestamp << " c=" << b.posConfidence <<
      " @ " << b.pos.x << ", " << b.pos.y
      << " with motion c=" << b.motionConfidence;
    if (b.motionConfidence > 0.01f) {
        s << ", dx,y=" << b.motion.x << ", " << b.motion.y;
    }
    return s;
}

std::ostream &operator<<(std::ostream &s, const Robot &r) {
    s << "Robot id=" << r.id << " t=" << r.timestamp << " c=" << r.confidence <<
      " @ " << r.pos.coord.x << ", " << r.pos.coord.y << ", " << r.pos.angle << "rad"
      << " GROUNDTRUTH: " << r.GTpos.coord.x << "," << r.GTpos.coord.y << "," <<
      r.GTpos.angle << ",Conf: " << r.GTconfidence << ", act=" << r.active;
    return s;
}


// vim: set ts=4 sw=4 sts=4 expandtab:
