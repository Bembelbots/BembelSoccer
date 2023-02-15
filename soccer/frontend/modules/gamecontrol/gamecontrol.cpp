#include <boost/asio.hpp>
#include <cstring>

#include "gamecontrol.h"

#include <framework/util/assert.h>
#include <framework/thread/util.h>
#include <framework/util/frame.h>

using boost::asio::ip::udp;
using boost::asio::ip::address_v4;

static_assert(NUM_PLAYERS == MAX_NUM_PLAYERS,
              "NUM_PLAYERS is not equal to max. number of players defined in SPL headers");

static void coord2array(float a[2], const Coord &c) {
    a[0] = c.x * 1000;
    a[1] = c.y * 1000;
}

Gamecontrol::Gamecontrol() :
    teamIndex(-1),
    lastReceived(-10000),
    isUnstiff(false),
    isPenalized(false) {
}

void Gamecontrol::connect(rt::Linker &link) {
    link.name = "Gamecontrol";
    link(bb);
    link(settings);
    link(output);
    link(body_state);
    link(world);
}

void Gamecontrol::setup() {
    std::memset(&gc_data, 0, sizeof(gc_data));
    gc_data.firstHalf = true;
    gc_data.secsRemaining = 600;

    for (size_t i{0}; i<sizeof(gc_return.header); ++i)
        gc_return.header[i] = GAMECONTROLLER_RETURN_STRUCT_HEADER[i];
    gc_return.version = GAMECONTROLLER_RETURN_STRUCT_VERSION;
    gc_return.playerNum = uint16_t(settings->id + 1);
    gc_return.teamNum = uint16_t(settings->teamNumber);
    // initialize team color in drop-in games, gamecontroller may override this
    net = std::make_shared<UDP>(Network::GAMECONTROL, &Gamecontrol::recv, this);
}

void Gamecontrol::process() {
    FrameScope frame(30ms);
    
    for(auto &body : body_state.fetch()) {
        LOG_DEBUG_IF(body.qns[CHEST_BUTTON_PRESSED]) << "Chest button pressed: " << std::this_thread::get_id();
        buttonHandler(body);
    }

    updateBB();
    *output = bb;
}

void Gamecontrol::recv(const char *msg, const size_t &bytes_recvd,
                       const udp::endpoint &sender) {
    struct RoboCupGameControlData recvBuf;
    static TimestampMs lastAlive = 0;
    static int prevPenalty = 0;
    int penalty = -1;

    // check if received packet is a gameserver struct
    if ((bytes_recvd != sizeof(recvBuf))
            || (std::strncmp(msg, GAMECONTROLLER_STRUCT_HEADER, 4) != 0)) {
        return;
    }

    std::memcpy(&recvBuf, msg, sizeof(recvBuf));

    int newTeamIndex = -1;
    for (int i = 0; i < 2; ++i) {
        if (recvBuf.teams[i].teamNumber == settings->teamNumber) {
            newTeamIndex = i;
            break;
        }
    }

    if (newTeamIndex < 0) {
        // gameserver didn't send any infos for our team
        // LOG_INFO << "received GameController packet from " << sender << " without info for team " << settings()->teamNumber;
        return;
    }

    static int lastPkt = -1;
    int pkt = recvBuf.packetNumber;
    if ((lastPkt > pkt) && (pkt > 10)) {
        return;
    }
    lastPkt = pkt;

    {
        std::scoped_lock lock(mtx);
        gc_data = recvBuf;
        teamIndex = newTeamIndex;
        lastReceived = getTimestampMs();
    }

    udp::endpoint ep(sender);
    ep.port(GAMECONTROLLER_RETURN_PORT);
  
    // send alive packet (approx. every 500ms)
    if ((getTimestampMs() - lastAlive) > 400) {
        auto &ball = (*world)->myBallPoseWcs;
        auto &robot = (*world)->allRobots[settings->id];
        
        // fill in pose fields in gamecontroller return message
        //gc_return.playerNum = settings->id + 1;
        //gc_return.teamNum = settings->teamNumber;

        gc_return.fallen = (robot.fallen > 0);
        coord2array(gc_return.pose, robot.pos.coord);
        gc_return.pose[2] = robot.pos.angle.rad();

        gc_return.ballAge = ball.age() / 1000.0f;
        coord2array(gc_return.ball, ball.pos);
        if (net) {
            net->sendTo(reinterpret_cast<char *>(&(gc_return)), sizeof(gc_return), ep);
        }
        lastAlive = getTimestampMs();
        //LOG_DEBUG << "sent alive packet for robot " << gc_return.player << "/" << gc_return.team << " to ", ep;
    }

    RobotInfo r = recvBuf.teams[newTeamIndex].players[settings->id];
    penalty = r.penalty;
    if (penalty != prevPenalty) {
        checkPenalty(false);
    }
    prevPenalty = penalty;
}


void Gamecontrol::updateBB() {
    auto lock = bb->scopedLock();
    static TimestampMs lastTs = -1;
   
    RoboCupGameControlData d;
    int idx;
    TimestampMs curTs;
    {
        std::scoped_lock lock(mtx);
        d = gc_data;
        idx = teamIndex;
        curTs = lastReceived;
    }

    // handle unstiff state change by either bembelDbug or button interface
    static bool wasUnstiff{false};
    if (isUnstiff != wasUnstiff)
        bb->unstiff = isUnstiff;    // our state changed, update blackboard
    else
        isUnstiff = bb->unstiff;    // our state has not changed, update from blackboard
    wasUnstiff = isUnstiff;


    static bool wasPenalized{false};
    static Penalty lastPenalty{Penalty::NONE};

    // manual penalty by bembelDbug
    if (lastPenalty != bb->penalty) { 
        if (bb->penalty == Penalty::MANUAL)
            checkPenalty(true);
        else if (bb->penalty == Penalty::NONE && lastPenalty == Penalty::MANUAL)
            checkPenalty(true);
    }

    if (isPenalized) {
        bb->penalty = Penalty::MANUAL;
    } else {
        // button interface: go to play state on unpenalize (gamecontroller will override if running)
        if ((bb->gameState == GameState::INITIAL) && wasPenalized)
            bb->gameState = GameState::PLAYING;
        bb->penalty = Penalty::NONE;
    }
    wasPenalized = isPenalized;
    lastPenalty = bb->penalty;

    settings->isPenaltyShootout = (settings->role == RobotRole::PENALTYKICKER) ||
                                    (settings->role == RobotRole::PENALTYGOALIE) ||
                                    (bb->gamePhase == GamePhase::PENALTYSHOOT);

    // only run when new packet has been received
    if (curTs <= lastTs) {
        return;
    }
    lastTs = curTs;

    int team = settings->teamNumber;

    bb->gamePhase = GamePhase(d.gamePhase);
    bb->gameState = GameState(d.state);
    bb->competitionType = CompetitionType(d.competitionType);
    bb->competitionPhase = CompetitionPhase(d.competitionPhase);
    bb->setPlay = SetPlay(d.setPlay);
    bb->firstHalf = (d.firstHalf > 0);
    bb->kickoff = (d.kickingTeam == team);

    settings->isPenaltyShootout = settings->isPenaltyShootout || (bb->gamePhase == GamePhase::PENALTYSHOOT);

    bb->secsRemaining = d.secsRemaining;
    bb->secondaryTime = d.secondaryTime;

    bb->teamNumber = team;

    if (idx >= 0) {
        RobotInfo r = d.teams[idx].players[settings->id];
        if (isPenalized && r.penalty) {
            bb->penalty = Penalty(r.penalty);
        }

        bb->secsTillUnpenalised = r.secsTillUnpenalised;

        auto newColor = TeamColor(d.teams[idx].teamColour);
        if (newColor != bb->teamColor) {
            LOG_INFO << "setting teamColor to " << int(bb->teamColor);
        }
        bb->teamColor = newColor;
        bb->score = d.teams[idx].score;
        bb->penaltyShot = d.teams[idx].penaltyShot;
        bb->singleShots = d.teams[idx].singleShots;
        bb->messageBudget = d.teams[idx].messageBudget;

        int opponent = 1 - idx;
        bb->opponentTeamNumber = d.teams[opponent].teamNumber;
        bb->opponentTeamColor = TeamColor(d.teams[opponent].teamColour);
        bb->opponentScore = d.teams[opponent].score;
        bb->opponentPenaltyShot = d.teams[opponent].penaltyShot;
        bb->opponentSingleShots = d.teams[opponent].singleShots;
        bb->opponentMessageBudget = d.teams[opponent].messageBudget;

        for (int i = 0; i < NUM_PLAYERS; ++i) {
            bb->penalties[i] = Penalty(d.teams[idx].players[i].penalty);
            bb->opponentPenalties[i] = Penalty(d.teams[opponent].players[i].penalty);
        }
    }

    bb->lastPacketTs = curTs;
}


void Gamecontrol::checkPenalty(bool manual) {
    RobotInfo r = gc_data.teams[teamIndex].players[settings->id];
    int penalty = r.penalty;

    if (manual) {
        if (isPenalized) {
            unpenalize();
        } else {
            penalize();
        }
    } else {
        if (isPenalized) {
            if (!penalty) {
                unpenalize();
            }
        } else {
            if (penalty) {
                penalize();
            }
        }
    }
}

void Gamecontrol::penalize() {
    isPenalized = true;
    LOG_SAY << "Penalized.";
    LOG_INFO << "penalized";
}

void Gamecontrol::unpenalize() {
    isPenalized = false;
    if (gc_data.state == STATE_INITIAL) {
        std::scoped_lock lock(mtx);
        gc_data.state = STATE_PLAYING;
    }

    LOG_SAY << "Unpenalized.";
    LOG_INFO << "unpenalized";
}

void Gamecontrol::buttonHandler(const BodyState &body) {
    static TimestampMs headTouchStart{-1};

    // check touch sensors on head for "unstiff" gesture
    if (body.qns[FRONT_HEAD_TOUCH_DOWN] 
        && body.qns[MIDDLE_HEAD_TOUCH_DOWN]
        && body.qns[REAR_HEAD_TOUCH_DOWN]) {
        if (headTouchStart < 0)
            headTouchStart = getTimestampMs();

        if (getTimestampMs() - headTouchStart > 1000 && !isUnstiff) {
            LOG_SAY << "Un-stiff";
            isUnstiff = true;
        }
    } else {
        headTouchStart = -1;
    }

    if (body.qns[CHEST_BUTTON_PRESSED]) {
        LOG_INFO << "ChestButton pressed.";
        if (isUnstiff) {
            LOG_SAY << "Re-stiff";
            isUnstiff = false;  // restiff if unstiffed
        } else {
            if ((getTimestampMs() - lastReceived) > 1000)
                checkPenalty(true); // handle chest button normally for penalizing
            else
                LOG_DEBUG << "Gamecontroller is running, ignoring ChestButton.";
        }
    }
}


// vim: set ts=4 sw=4 sts=4 expandtab:
