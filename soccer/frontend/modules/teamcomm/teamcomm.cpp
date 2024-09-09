#include <memory>
#include <string>
#include <cstring>
#include <algorithm>

#include <flatbuffers/verifier.h>

#include "gc_enums_generated.h"
#include "teamcomm.h"
#include "botnames_generated.h"
#include "roles_generated.h"
#include "team_message_generated.h"

#include <framework/logger/logger.h>
#include <framework/util/assert.h>
#include <framework/thread/util.h>

using namespace bbapi;

static constexpr uint8_t SPL_MSG_LIMIT{128};
static constexpr uint8_t MAX_OBSTACLES{8};

static bbapi::dpos coord2pos(const DirectedCoord &c) {
    return {static_cast<int16_t>(c.coord.x * 1000), static_cast<int16_t>(c.coord.y * 1000),
        static_cast<int16_t>(c.angle.rad() * 1000)};
}

static bbapi::pos coord2pos(const Coord &c) {
    return {static_cast<int16_t>(c.x * 1000), static_cast<int16_t>(c.y * 1000)};
}

static DirectedCoord pos2coord(const bbapi::dpos &p) {
    return {p.x() / 1000.f, p.y() / 1000.f, Rad{p.a() / 1000.f}};
}

static Coord pos2coord(const bbapi::pos &p) {
    return {p.x() / 1000.f, p.y() / 1000.f};
}


void TeamComm::connect(rt::Linker &link) {
    link.name = "TeamComm";
    link(settings);
    link(gamecontrol);
    link(world);
    link(cmds);
    link(team_message);
    link(team_message_log);
    link(debugState);
    link(body);
    link(whistle);
    link(refereeGesture);
}

void TeamComm::setup() {
    net = std::make_shared<UDP>(Network::SPL_MSG, &TeamComm::netRecv, this, settings->teamNumber);
    cmds.connect<TeamcommDebugInfo, &TeamComm::handle>(this);
    
    tm.position = std::make_unique<bbapi::dpos>();
    tm.ball = std::make_unique<bbapi::pos>();
    tm.teamBall = std::make_unique<bbapi::pos>();
    tm.walktarget = std::make_unique<bbapi::dpos>();
    tm.goaltarget = std::make_unique<bbapi::pos>();
}

void TeamComm::process() {
    // do not broadcast anything during penalty shootouts, in accordance with SPL rules
    if (gamecontrol->gamePhase != bbapi::GamePhase::PENALTYSHOOT)
        broadcast((*world)->myRobotPoseWcs, (*world)->myBallPoseRcs);
    sleep_for(30ms);
}

void TeamComm::broadcast(const Robot &r, const Ball &b) {
    const auto &wm = *world; 
    const Ball &teamball = wm->teamball;

    tm.name = settings->name;
    tm.playerNum = settings->id + 1;
    tm.role = debug.role;
    tm.fallen = (r.fallen > 0);
    tm.battery = body->sensors.battery.charge * 100;
    
    *tm.position = coord2pos(r.pos);
    *tm.ball = coord2pos(b.pos);
    tm.ballAge = b.age();
    tm.ballConf = b.posConfidence * 100;

    *tm.teamBall = coord2pos(teamball.pos);
    tm.teamBallConf = teamball.posConfidence * 100;
    tm.isNearestToBall = wm->iAmNearestToTeamBall;

    *tm.walktarget = coord2pos(debug.walktarget_pos);
    *tm.goaltarget = coord2pos(debug.dribble_target);

    tm.obstacles.clear();
    for (const auto &o: wm->detectedRobots)
        tm.obstacles.emplace_back(coord2pos(o.pos.coord));

    tm.refereeGestureUp = false;
    for (const auto &gesture : refereeGesture.fetch())
        tm.refereeGestureUp |= (gesture.leftArmUp and gesture.rightArmUp);

    flatbuffers::FlatBufferBuilder builder;
    auto packed = bbapi::TeamMessage::Pack(builder, &tm);

    builder.FinishSizePrefixed(packed);
    TimestampMs now = getTimestampMs();
    
    // send teamcomm updates every 2s to last seen bembelDbug
    static TimestampMs lastDebug = 0;
    using namespace boost::asio::ip;
    auto debug_ep = udp::endpoint(address::from_string("10.0.3.1"), 10000+settings->teamNumber);

    if ((now - lastDebug) > 2100) {
        net->sendTo(reinterpret_cast<char *>(builder.GetBufferPointer()), builder.GetSize(), debug_ep);
        lastDebug = now;
    }

    static TimestampMs lastBCast = 0;
    auto interval = calcInterval(r, b);
    if ((interval > 0 && (now - lastBCast) < interval)
        || gamecontrol->unstiff 
        || gamecontrol->penalized)
        return;

    lastBCast = now;

    if (gamecontrol->myTeamInfo && (std::min(static_cast<int>(gamecontrol->myTeamInfo->messageBudget), 1200-msgCount) < 3*numPlayers)) {
        LOG_WARN << "TeamComm: message budget low, not sending team message.";
        return;
    }

    if (builder.GetSize() > SPL_MSG_LIMIT) {
        LOG_ERROR << "TeamComm: TeamMessage exceed allowed message size (size=" << builder.GetSize() << ", limit=" << SPL_MSG_LIMIT << "), not sending packet!" ;
        return;
    }
    net->bcast(reinterpret_cast<char *>(builder.GetBufferPointer()), builder.GetSize(), Network::SPL_MSG);
    ++msgCount;
}

void TeamComm::handle(TeamcommDebugInfo &reactivewalk) {
    debug = reactivewalk;
}

void TeamComm::netRecv(const char *data, const size_t &bytes_recvd,
                       const udp::endpoint &sender) {
    bbapi::TeamMessageT msg;
    flatbuffers::Verifier v(reinterpret_cast<const unsigned char*>(data), bytes_recvd);
    if (!bbapi::VerifySizePrefixedTeamMessageBuffer(v)) {
        LOG_ERROR << "TeamComm: received invalid flatbuffer data from " << sender;
        return;
    }
    bbapi::GetSizePrefixedTeamMessage(data)->UnPackTo(&msg);

    int senderID = msg.playerNum - 1;

    if (senderID == settings->id)
        return;

    if ((senderID < 0) || (senderID >= numPlayers)) {
        LOG_WARN << "TeamComm: received packet from invalid id " << senderID << " from " << sender;
        return;
    }

    msgTimestamp[senderID] = getTimestampMs();
    ++msgCount;

    std::vector<Coord> obstacles;
    for (const auto &o: msg.obstacles)
        obstacles.emplace_back(pos2coord(o));

    std::optional<Ball> ball;
    if (msg.ball)
        ball = teamMsg2ball(senderID, *msg.ball, msg.ballConf, msg.ballAge);

    std::optional<Ball> team_ball;
    if (msg.teamBall)
        team_ball = teamMsg2ball(senderID, *msg.teamBall, msg.teamBallConf);

    std::optional<DirectedCoord> walktarget;
    if (msg.walktarget)
        walktarget = pos2coord(*msg.walktarget);

    std::optional<Coord> goaltarget;
    if (msg.goaltarget)
        goaltarget = pos2coord(*msg.goaltarget);

    team_message.emit({
                        .robot = teamMsg2robot(msg),
                        .ball = ball,
                        .team_ball = team_ball,
                        .nearest_to_team_ball = msg.isNearestToBall,
                        .walktarget = walktarget,
                        .goaltarget = goaltarget,
                        .obstacles = obstacles,
                        .timestamp = msgTimestamp[senderID],
                        .refereeGestureUp = msg.refereeGestureUp,
                    });

    team_message_log.emit(msg);
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

    // received ack within the last ~20s
    return msgTimestamp[id] > (getTimestampMs() - 20000);
}

Robot TeamComm::teamMsg2robot(const bbapi::TeamMessageT &msg) {
    Robot r;
    r.id = msg.playerNum - 1;
    r.fallen = msg.fallen;
    r.active = isActive(r.id);
    const auto &ti{gamecontrol->myTeamInfo};
    if (ti && (static_cast<size_t>(r.id) < ti->players.size()) && (r.id >= 0))
        r.active &= (ti->players[r.id]->penalty == Penalty::NONE);

    r.timestamp = msgTimestamp[r.id];
    if (msg.position)
        r.pos = pos2coord(*msg.position);

    r.confidence = msg.posConf / 100.f;

    r.role = static_cast<RobotRole>(msg.role);
    return r;
}

Ball  TeamComm::teamMsg2ball(const int &sender, const bbapi::pos &pos, const uint8_t &conf, const int &age) {
    Ball b;

    b.id = sender;
    b.pos = pos2coord(pos);

    if (age > 0)
        b.timestamp = msgTimestamp[sender] - age;
    else
        b.timestamp = -1;
    b.posConfidence = conf / 100.f;
    b.motionConfidence = 0;

    return b;
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

    auto state = gamecontrol->gameState;
    if (lastState != state || w)
        interval = immediateInterval;
    else if (tm.refereeGestureUp)
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
