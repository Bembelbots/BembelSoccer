#include <memory>
#include <cstring>
#include <boost/asio.hpp>

#include <framework/util/assert.h>
#include <framework/thread/util.h>
#include <framework/util/frame.h>

#include "gamecontrol.h"
#include "gamecontrol_generated.h"
#include "gc_enums_generated.h"

using boost::asio::ip::address_v4;
using boost::asio::ip::udp;

using namespace bbapi;

static_assert(
        NUM_PLAYERS == MAX_NUM_PLAYERS, "NUM_PLAYERS is not equal to max. number of players defined in SPL headers");

static_assert(GAMECONTROLLER_STRUCT_VERSION == static_cast<int>(GCConst::STRUCT_VERSION));

static void coord2array(float a[2], const Coord &c) {
    a[0] = c.x * 1000;
    a[1] = c.y * 1000;
}

Gamecontrol::Gamecontrol() : teamIndex(-1) {
}

void Gamecontrol::connect(rt::Linker &link) {
    link.name = "Gamecontrol";
    link(bb);
    link(settings);
    link(gc_event);
    link(body_state);
    link(world);
}

void Gamecontrol::setup() {
    gc_data.gameState = GameState::INITIAL;
    gc_data.myPenaltyState = std::make_unique<bbapi::RobotInfoT>();

    gc_data.firstHalf = true;
    gc_data.secsRemaining = 600;

    for (size_t i{0}; i < sizeof(gc_return.header); ++i)
        gc_return.header[i] = GAMECONTROLLER_RETURN_STRUCT_HEADER[i];
    gc_return.version = GAMECONTROLLER_RETURN_STRUCT_VERSION;
    gc_return.playerNum = uint16_t(settings->id + 1);
    gc_return.teamNum = uint16_t(settings->teamNumber);
    // initialize team color in drop-in games, gamecontroller may override this
    net = std::make_shared<UDP>(Network::GAMECONTROL, &Gamecontrol::recv, this);
}

void Gamecontrol::process() {
    FrameScope frame(30ms);
    static bbapi::GamecontrolMessageT gc_last;
    std::scoped_lock gcLock(mtx);

    for (auto &body : body_state.fetch())
        buttonHandler(body);

    updateBB();

    if (gc_last != gc_data) {
        // override role for penalty shootout
        // FIXME: this shouldn't be handled here, but currently that's the easiest fix
        if (gc_data.gamePhase == GamePhase::PENALTYSHOOT) {
            if (gc_data.kickoff)
                settings->role = RobotRole::PENALTYKICKER;
            else
                settings->role = RobotRole::PENALTYGOALIE;
        }

        if (gc_data.penalized != gc_last.penalized) {
            if (gc_data.penalized) {
                LOG_SAY << "Penalized.";
            } else {
                LOG_SAY << "Unpenalized.";
            }
        }
        gc_event.emit(gc_data);
        gc_last = gc_data;
    }
}

void Gamecontrol::recv(const char *msg, const size_t &bytes_recvd, const udp::endpoint &sender) {
    struct RoboCupGameControlData recvBuf;
    static TimestampMs lastAlive = 0;
    auto prevPenalty{gc_data.myPenaltyState->penalty};

    // check if received packet is a gameserver struct
    if ((bytes_recvd != sizeof(recvBuf)) || (std::strncmp(msg, GAMECONTROLLER_STRUCT_HEADER, 4) != 0)) {
        return;
    }

    std::memcpy(&recvBuf, msg, sizeof(recvBuf));

    if (recvBuf.version != GAMECONTROLLER_STRUCT_VERSION) {
        LOG_WARN << __PRETTY_FUNCTION__ << " - received incompatible GameController message (got "
                 << int(recvBuf.version) << ", expected " << GAMECONTROLLER_STRUCT_VERSION << ")";
        return;
    }

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
    if ((gc_data.packetNumber > pkt) && (pkt > 10))
        return;
    lastPkt = pkt;

    teamIndex = newTeamIndex;
    gc_data.lastPacketTs = getTimestampMs();
    parsePacket(recvBuf);

    udp::endpoint ep(sender);
    ep.port(GAMECONTROLLER_RETURN_PORT);

    // send alive packet (approx. every 500ms)
    if ((getTimestampMs() - lastAlive) > 500) {
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
        net->sendTo(reinterpret_cast<char *>(&(gc_return)), sizeof(gc_return), ep);
        lastAlive = getTimestampMs();
        //LOG_DEBUG << "sent alive packet for robot " << gc_return.player << "/" << gc_return.team << " to ", ep;
    }
}

void Gamecontrol::updateBB() {
    auto bbLock = bb->scopedLock();

    // handle unstiff state change by either bembelDbug or button interface
    static bool wasUnstiff{false};
    if (gc_data.unstiff != wasUnstiff)
        bb->unstiff = gc_data.unstiff; // our state changed, update blackboard
    else
        gc_data.unstiff = bb->unstiff; // our state has not changed, update from blackboard
    wasUnstiff = gc_data.unstiff;

    static bool wasPenalized{false};
    static int lastPenalty{0};

    // manual penalty by bembelDbug
    if (lastPenalty != bb->penalty) {
        gc_data.myPenaltyState->penalty = static_cast<Penalty>(bb->penalty);
    }
    lastPenalty = bb->penalty;

    gc_data.penalized = (gc_data.myPenaltyState->penalty != Penalty::NONE);
    bb->penalty = static_cast<int>(gc_data.myPenaltyState->penalty);
    bb->penalty = gc_data.penalized;

    static int lastState{0};
    if (bb->gameState != lastState)
        gc_data.gameState = static_cast<GameState>(bb->gameState);
    else
        bb->gameState = static_cast<int>(gc_data.gameState);
    lastState = bb->gameState;

    bb->gamePhase = static_cast<int>(gc_data.gamePhase);
    bb->competitionType = static_cast<int>(gc_data.competitionType);
    bb->competitionPhase = static_cast<int>(gc_data.competitionPhase);
    bb->setPlay = static_cast<int>(gc_data.setPlay);
    bb->firstHalf = gc_data.firstHalf;
    bb->kickoff = gc_data.kickoff;

    bb->secsRemaining = gc_data.secsRemaining;
    bb->secondaryTime = gc_data.secondaryTime;

    bb->penalty = static_cast<int>(gc_data.myPenaltyState->penalty);
    bb->secsTillUnpenalised = gc_data.myPenaltyState->secsUntilUnpenalized;

    bb->lastPacketTs = gc_data.lastPacketTs;
}

void Gamecontrol::parsePacket(const RoboCupGameControlData &pkt) {
    std::scoped_lock lock(mtx);

    gc_data.gcVersion = pkt.version;
    gc_data.packetNumber = pkt.packetNumber;
    gc_data.playersPerTeam = pkt.playersPerTeam;

    jsassert(pkt.competitionPhase <= static_cast<uint8_t>(bbapi::CompetitionPhase::MAX));
    gc_data.competitionPhase = EnumValuesCompetitionPhase()[pkt.competitionPhase];

    jsassert(pkt.competitionType <= static_cast<uint8_t>(bbapi::CompetitionType::MAX));
    gc_data.competitionType = EnumValuesCompetitionType()[pkt.competitionType];

    jsassert(pkt.gamePhase <= static_cast<uint8_t>(bbapi::GamePhase::MAX));
    gc_data.gamePhase = EnumValuesGamePhase()[pkt.gamePhase];

    jsassert(pkt.state <= static_cast<uint8_t>(bbapi::GameState::MAX));
    gc_data.gameState = EnumValuesGameState()[pkt.state];

    jsassert(pkt.setPlay <= static_cast<uint8_t>(bbapi::SetPlay::MAX));
    gc_data.setPlay = EnumValuesSetPlay()[pkt.setPlay];

    gc_data.firstHalf = pkt.firstHalf;
    gc_data.kickingTeam = pkt.kickingTeam;
    gc_data.kickoff = pkt.kickingTeam == settings->teamNumber;

    gc_data.secsRemaining = pkt.secsRemaining;
    gc_data.secondaryTime = pkt.secondaryTime;

    if (!gc_data.myTeamInfo || !gc_data.opponentTeamInfo) {
        // create TeamInfo objects when first message has been received
        gc_data.myTeamInfo = std::make_unique<bbapi::TeamInfoT>();
        gc_data.opponentTeamInfo = std::make_unique<bbapi::TeamInfoT>();
        for (int i{0}; i < MAX_NUM_PLAYERS; ++i) {
            gc_data.myTeamInfo->players.emplace_back(std::make_unique<bbapi::RobotInfoT>());
            gc_data.opponentTeamInfo->players.emplace_back(std::make_unique<bbapi::RobotInfoT>());
        }
    }

    for (const auto &team : pkt.teams) {
        auto &t{(team.teamNumber == settings->teamNumber) ? *(gc_data.myTeamInfo.get())
                                                          : *(gc_data.opponentTeamInfo.get())};

        t.teamNumber = team.teamNumber;

        jsassert(team.fieldPlayerColour <= static_cast<uint8_t>(bbapi::TeamColor::MAX));
        t.fieldPlayerColor = EnumValuesTeamColor()[team.fieldPlayerColour];

        jsassert(team.goalkeeperColour <= static_cast<uint8_t>(bbapi::TeamColor::MAX));
        t.goalkeeperColor = EnumValuesTeamColor()[team.goalkeeperColour];
        t.goalkeeper = team.goalkeeper;

        t.score = team.score;
        t.penaltyShot = team.penaltyShot;
        t.singleShots = team.singleShots;
        t.messageBudget = team.messageBudget;

        jsassert(t.players.size() == MAX_NUM_PLAYERS);
        jsassert(sizeof(team.players) == MAX_NUM_PLAYERS * sizeof(struct ::RobotInfo));
        for (int i{0}; i < MAX_NUM_PLAYERS; ++i) {
            const auto &p = team.players[i];
            t.players.at(i)->penalty = static_cast<bbapi::Penalty>(p.penalty);
            t.players.at(i)->secsUntilUnpenalized = p.secsTillUnpenalised;
        }
    }

    *gc_data.myPenaltyState = *gc_data.myTeamInfo->players.at(settings->id);
    gc_data.penalized = (gc_data.myPenaltyState->penalty != Penalty::NONE);
}

void Gamecontrol::buttonHandler(const BodyState &body) {
    static TimestampMs headTouchStart{-1};

    // check touch sensors on head for "unstiff" gesture
    if (body.qns[FRONT_HEAD_TOUCH_DOWN] && body.qns[MIDDLE_HEAD_TOUCH_DOWN] && body.qns[REAR_HEAD_TOUCH_DOWN]) {
        if (headTouchStart < 0)
            headTouchStart = getTimestampMs();

        if (getTimestampMs() - headTouchStart > 1000 && !gc_data.unstiff) {
            LOG_SAY << "Un-stiff";
            gc_data.unstiff = true;
        }
    } else {
        headTouchStart = -1;
    }

    if (body.qns[CHEST_BUTTON_PRESSED]) {
        LOG_INFO << "ChestButton pressed.";
        if (gc_data.unstiff) {
            LOG_SAY << "Re-stiff";
            gc_data.unstiff = false; // restiff if unstiffed
        } else {
            if ((getTimestampMs() - gc_data.lastPacketTs) < 1000) {
                LOG_DEBUG << "Gamecontroller is running, ignoring ChestButton.";
            } else { // handle chest button normally for penalizing
                if (gc_data.penalized) {
                    gc_data.myPenaltyState->penalty = Penalty::NONE;
                    gc_data.gameState = GameState::PLAYING;
                } else {
                    gc_data.myPenaltyState->penalty = Penalty::MANUAL;
                }
                gc_data.penalized = (gc_data.myPenaltyState->penalty != Penalty::NONE);
            }
        }
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:
