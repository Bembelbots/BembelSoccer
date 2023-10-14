#include <memory>
#include <string>
#include <cstring>
#include <algorithm>

#include <flatbuffers/verifier.h>

#include "teamcomm.h"
#include "botnames_generated.h"
#include "roles_generated.h"
#include "team_message_generated.h"

#include <framework/logger/logger.h>
#include <framework/util/assert.h>
#include <framework/thread/util.h>

static constexpr uint8_t SPL_MSG_LIMIT{128};
static constexpr uint8_t MAX_OBSTACLES{8};

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
    cmds.connect<TeamcommDebugInfo, &TeamComm::handle>(this);
}

void TeamComm::process() {
    cmds.update();

    // do not broadcast anything during penalty shootouts, in accordance with SPL rules
    if (!settings->isPenaltyShootout) {
        broadcast((*world)->myRobotPoseWcs, (*world)->myBallPoseRcs);
    }
    sleep_for(30ms);
}

void TeamComm::broadcast(const Robot &r, const Ball &b) {
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

    int senderID{0};

    if (senderID == settings->id)
        return;

    if ((senderID < 0) || (senderID >= numPlayers)) {
        LOG_WARN << "TeamComm: received packet from invalid id " << senderID << " from " << sender;
        return;
    }

    msgTimestamp[senderID] = getTimestampMs();
    ++msgCount;

    team_message.emit({
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

    // received ack within the last ~20s
    return msgTimestamp[id] > (getTimestampMs() - 20000);
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
