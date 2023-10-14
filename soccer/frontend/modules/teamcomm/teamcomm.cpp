#include "teamcomm.h"
#include <string>
#include <cstring>
#include <algorithm>

#include <framework/logger/logger.h>
#include <framework/util/assert.h>
#include <framework/thread/util.h>

static constexpr uint32_t BB_MAGIC = 0xBADC0DE;

static void coord2array(float a[2], const Coord &c) {
    a[0] = c.x * 1000;
    a[1] = c.y * 1000;
}

static Coord array2coord(const float a[2]) {
    return {a[0] / 1000.0f, a[1] / 1000.0f};
}

TeamComm::TeamComm() :
    seq(),
    ack() {
    int i = 0;
    for (auto &m: msg) {
        m.playerNum = ++i;
    }
}

TeamComm::~TeamComm() {

}

void TeamComm::connect(rt::Linker &link) {
    link.name = "TeamComm";
    link(settings);
    link(gc_message);
    link(world);
    link(cmds);
    link(team_message);
    link(debugState);
    link(body);
    link(whistle);
}

void TeamComm::setup() {
    net = std::make_shared<UDP>(Network::SPL_MSG, &TeamComm::netRecv, this, settings->teamNumber);
}

void TeamComm::process() {
    // do not broadcast anything during penalty shootouts, in accordance with SPL rules
    if (!settings->isPenaltyShootout) {
        broadcast((*world)->myRobotPoseWcs, (*world)->myBallPoseRcs);
    }
    sleep_for(30ms);
}

void TeamComm::broadcast(const Robot &r, const Ball &b) {
    //static BlackboardIO &bbio{*tafel().getBlackboardIO()};

    // construct SPL message
    SPLStandardMessage spl;

    // fill in the standard SPL data
    spl.playerNum = settings->id + 1;
    spl.teamNum = settings->teamNumber;

    spl.fallen = (r.fallen > 0);
    coord2array(spl.pose, r.pos.coord);
    spl.pose[2] = r.pos.angle.rad();

    spl.ballAge = b.age() / 1000.0f;
    coord2array(spl.ball, b.pos);

    spl.numOfDataBytes = 0;

    TimestampMs now = getTimestampMs();
    
    // send teamcomm updates every 2s to last seen bembelDbug
    static TimestampMs lastDebug = 0;
    auto debug_ep = debugState->debug_ep;
    debug_ep.port(10000+settings->teamNumber);

    // ignore address 0.0.0.0
    if (debug_ep.address() != boost::asio::ip::address_v4() && (now - lastDebug) > 2100) {
        net->sendTo(reinterpret_cast<char *>(&spl), sizeof(spl), debug_ep);
        lastDebug = now;
    }

    auto &gc = *gc_message;

    static TimestampMs lastBCast = 0;
    auto interval = calcInterval(r, b);
    if ((interval > 0 && (now - lastBCast) < interval)
        || gc->unstiff 
        || gc->penalty != Penalty::NONE)
        return;

    lastBCast = now;

    if (std::min(gc->messageBudget, 1200-msgCount) < 3*numPlayers) {
        LOG_WARN << "TeamComm: message budget low, not sending team message.";
        return;
    }

    net->bcast(reinterpret_cast<char *>(&spl), sizeof(spl), Network::SPL_MSG);
    ++msgCount;
}

void TeamComm::netRecv(const char *data, const size_t &bytes_recvd,
                       const udp::endpoint &sender) {
    const SPLStandardMessage &spl = reinterpret_cast<const SPLStandardMessage &>
                                    (*data);
    int senderID = spl.playerNum - 1;

    // sanity checks
    if (bytes_recvd < sizeof(struct SPLStandardMessage)) // invalid size
        return;
    if (std::string(spl.header,
                    4) != SPL_STANDARD_MESSAGE_STRUCT_HEADER) // wrong header
        return;
    if (spl.version != SPL_STANDARD_MESSAGE_STRUCT_VERSION) // wrong version
        return;
    if (spl.teamNum != settings->teamNumber) // wrong team
        return;
    if (senderID == settings->id) // own broadcasts
        return;
    if ((senderID < 0) || (senderID >= numPlayers)) // invalid player number
        return;

    auto robot = spl2robot(spl);
    
    Ball ball_rcs;
    ball_rcs.id = spl.playerNum - 1;
    ball_rcs.pos = array2coord(spl.ball);
    if (spl.ballAge >= 0) {
        int age = getTimestampMs() - msgTimestamp[ball_rcs.id] + (spl.ballAge * 1000.0f);
        // fake confidences
        ball_rcs.posConfidence = std::max(0.0f, 1.0f - (age / 10000.0f));   // 0 after 10s
        ball_rcs.motionConfidence = std::max(0.0f, 1.0f - (age / 1000.0f)); // 0 after 1s
        ball_rcs.timestamp = getTimestampMs() - age;
    } else {
        ball_rcs.posConfidence = ball_rcs.motionConfidence = 0;
        ball_rcs.timestamp = INT_MIN;
    }

    team_message.emit(TeamMessage {
                        .robot = robot,
                        .ball = ball_rcs,
                        .timestamp = msgTimestamp[senderID]
                    });
}

bool TeamComm::isActive(const int &id) {
    // we are always active
    if (id == settings->id)
        return true;

    // someone did something nasty
    if ((id < 0) || (id >= numPlayers)) {
        LOG_ERROR << __func__ << " called with invalid player ID " << id;
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(mtx);

    // received ack within the last ~20s
    return msgTimestamp[id] > (getTimestampMs() - 20000);
}

Robot TeamComm::spl2robot(const SPLStandardMessage &spl) {
    Robot r;
    r.id = spl.playerNum - 1;
    r.fallen = spl.fallen;
    r.active = isActive(r.id) && ((*gc_message)->penalties[r.id] == Penalty::NONE);

    r.timestamp = getTimestampMs() - msgTimestamp[r.id];
    r.pos.coord = array2coord(spl.pose);
    r.pos.angle = Rad{spl.pose[2]};

    if (r.active)
        // FIXME: send confidence in BembelMessage
        r.confidence = 1.0f;
    else
        // inactive robots get 0 confidence
        r.confidence = 0.0f;

    return r;
}

float TeamComm::calcInterval(const Robot &r, const Ball &b) {
    static Robot lastRobot;
    static Ball lastBall;
    static GameState lastState{GameState::INITIAL};
    static bool lastNearest2ball{false}, lastWhistle{false};
    float interval{defaultBcastIntervalMs};

    // find minimum ball age of teammates
    TimestampMs minBallAge{100000};
    for (const auto &b: (*world)->allBallPoseWcs)
        if (b.age() != r.id)
            minBallAge = std::min(b.age(), minBallAge);

    bool w{false};
    for (const auto &e: whistle.fetch())
        w |= e.found;

    bool fallen{r.fallen > 0};
    float ballDist{b.pos.dist(r.pos.coord)};
    bool nearest = (*world)->iAmNearestToOwnBall;

    auto state = (*gc_message)->gameState;
    if (lastState != state || w)
        interval = immediateInterval;
    else if (state != GameState::PLAYING && !w)
        interval = defaultBcastIntervalMs;
    else if (fallen != (lastRobot.fallen > 0)) // tell teammates if fallen state changed
        interval = mediumInterval;
    else if (fallen) // no need to waste packets when fallen
        interval = defaultBcastIntervalMs;
    else if (lastNearest2ball && !nearest)
        interval = immediateInterval;
    else if (b.age() < thresholdBallAge) { // ball seen
        if ((ballDist < thresholdBallDist) && (b.pos.dist(lastBall.pos) > thresholdBallPosition))
            // ball is close and has moved
            interval = fastInterval;
        else if (minBallAge > thresholdTeamBallAge)
            // ball is not close, but no team but no teammate has seen it
            interval = mediumInterval;
    } else if (r.pos.coord.dist(lastRobot.pos.coord) > thresholdMovement) 
        // announce position more often while walking
        interval = mediumInterval;

    lastState = state;
    lastRobot = r;
    lastBall = b;
    lastWhistle = w;
    return interval;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
