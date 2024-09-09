#include "pose.h"

#include "gc_enums_generated.h"
#include "poseblackboard.h"
#include "hypothesesgenerator.h"

#include <framework/logger//logger.h>
#include <framework/math/directed_coord.h>
#include <framework/thread/util.h>
#include <framework/util/frame.h>

#include <representations/playingfield/playingfield.h>
#include <representations/worldmodel/definitions.h>

#include <boost/math/constants/constants.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <bitset>

#include <unordered_map>
#include <vector>

/// constant for sleeps in poseestimate (ms)
static const int POSEESTIMATE_WORKER_SLEEP_INTERVAL_MS = 200;

/// the minimum threshold both gyros must surpasse until the robot is assuming a stand up state
static const float MIN_GYRO_AMPLITUDE = 0.05;

Pose::Pose() :
    locaHypoGenerator(NULL),
    loca(NULL),
    _beforestandUP(0),
    _lastStateOfLostGroundContact(false),
    _lastStateOfIsPenalized(false),
    _isPenalizedCounter(0),
    _lostGroundContactCounter(0) {
    // get dedicated pose blackboard pointer
    poseBlackboard = std::make_shared<PoseBlackboard>();
}

void Pose::connect(rt::Linker &link) {
    link.name = "localization";
    link(settings);
    link(playingfield);
    link(body);
    link(gamecontrol);
    link(visionResults);
    link(locamessage);
}

void Pose::setup() {
    locaHypoGenerator = std::make_shared<HypothesesGenerator>(playingfield);   

    // CHANGE THIS TO REPLACE THE LOCALIZATION !!!
    ParticleFilter::Settings locaConf(playingfield);

    sensor_angle_offset = 0.0f;
    locaConf.robot_id = settings->id;
    locaConf.has_kickoff = gamecontrol->kickoff;
    locaConf.role = settings->role;
    locaConf.pf= playingfield;

    // we have to set all the configuration for the filter here!
    //
    loca = new ParticleFilter(locaConf);  
    locaEmitEvent(ParticleFilter::EV_INTIAL);

    _robotWcs.id = settings->id;
    _robotWcs.timestamp = getTimestampMs();

    _robotWcs.setUnknownPose();

    _robotWcs.GTtimestamp = getTimestampMs();
    _robotWcs.GTconfidence = 0.0f;

    _wasFallen = false;

}

void Pose::process() {
    auto lock = poseBlackboard->scopedLock();
    //GET ODOMETRY;use als sensors additional to walk output
    DirectedCoord dc = body->odometry;
    dc.angle = Rad{body->bodyAngles(2) - sensor_angle_offset};
    // odometry logger
    //LOG_DATA << "Odometry: " << dc.coord.x << ";" << dc.coord.y << ";" << dc.angle.rad();

    //CREATE HYPOTHESES
    std::pair<std::vector<DirectedCoord>,int> hypos = locaHypoGenerator->createHypotheses(visionResults);
    //outpus measurement( pose estimations from landmarks)  
    for (auto hypo: hypos.first){
        Robot r;
        r.pos =  hypo;
        r.confidence = hypos.second;
        //LOG_DATA<< "Measurement: "<< r;
    }
   
    //UPDATE LOCA
    loca->update(visionResults, dc,hypos);
   
    // update role for penalty shootout
    if (settings->role == RobotRole::PENALTYKICKER)
        locaEmitEvent(ParticleFilter::EV_ROLE_PENALTYKICKER);
    else if (settings->role == RobotRole::PENALTYGOALIE)
        locaEmitEvent(ParticleFilter::EV_ROLE_PENALTYGOALIE);

    //HAS GAMESTATE CHANGED
    bbapi::GameState gs = gamecontrol->gameState;
    static auto lastState = bbapi::GameState::FINISHED;
    if (lastState != gs) {
        using l = ParticleFilter;
        using bbapi::GameState;
        static const std::unordered_map<GameState, l::tLocalizationEvent> ev{
            {GameState::INITIAL, l::EV_STATE_INITIAL},
            {GameState::STANDBY, l::EV_STATE_STANDBY},
            {GameState::READY, l::EV_STATE_READY},
            {GameState::SET, l::EV_STATE_SET},
            {GameState::PLAYING, l::EV_STATE_PLAYING},
            {GameState::FINISHED, l::EV_STATE_FINISHED},
        };
        if (ev.count(gs) > 0)
            locaEmitEvent(ev.at(gs));
    }
    lastState = gs;

    
    //IS ROBOT FALLEN
    bool isFallen_curr = body->qns[IS_FALLEN] || body->qns[IS_STANDING_UP];
    if (isFallen_curr and ! _wasFallen) {
        _wasFallen = true;
        locaEmitEvent(ParticleFilter::EV_FALLEN);
    } else {
        if (_wasFallen and !isFallen_curr) {
            _wasFallen = false;
            locaEmitEvent(ParticleFilter::EV_BACK_UP);
        }
    }

    //CHECKS IF GROUNDCONTACT
    bool lostGroundContact = !body->qns[HAS_GROUND_CONTACT];
    if (lostGroundContact) {
        ++_lostGroundContactCounter;
        if (!_lastStateOfLostGroundContact) {
            if (_lostGroundContactCounter * POSEESTIMATE_WORKER_SLEEP_INTERVAL_MS > 1000) {
                _lastStateOfLostGroundContact = true;
                //LOG_INFO << "lost ground";
                locaEmitEvent(ParticleFilter::EV_LOST_GROUND);
            }
        }
    } else {
        _lostGroundContactCounter = 0;
        if (_lastStateOfLostGroundContact) {
            _lastStateOfLostGroundContact = false;
            locaEmitEvent(ParticleFilter::EV_GOT_GROUND);
        }
    }

    // CHECK IF PENALIZED
    if (gamecontrol->penalized) {
        ++_isPenalizedCounter;
        if (!_lastStateOfIsPenalized) {
            if (_isPenalizedCounter * POSEESTIMATE_WORKER_SLEEP_INTERVAL_MS > 1000) {
                _lastStateOfIsPenalized = true;
                locaEmitEvent(ParticleFilter::EV_PENALIZED);
            }
        }
    } else {
        _isPenalizedCounter = 0;
        if (_lastStateOfIsPenalized) {
            _lastStateOfIsPenalized = false;
            locaEmitEvent(ParticleFilter::EV_UNPENALIZED);
        }
    }

    // push hypotheses/particles to blackboard for debugging
    if (DEBUG_ON(poseBlackboard->Particles)) {
        poseBlackboard->Particles = loca->getHypothesesVector();
    }
    /*if (DEBUG_ON(_poseBlackboard->Hypotheses)) {
        _poseBlackboard->Hypotheses = loca->debuggingPos;
    }*/
    
    // add ground truth data, if available and not too old.
    bbapi::LocalizationMessageT message;
    message.pose        = loca->get_position();
    message.confidence  = loca->get_confidence();
    message.gtTimestamp = poseBlackboard->gtTimestamp;
    message.gtPosition  = {};

    if ((poseBlackboard->gtTimestamp > 0)
            && ((getTimestampMs() - poseBlackboard->gtTimestamp) < 5000)) {
        message.gtPosition = poseBlackboard->gtPosition;
    }
    
    *locamessage = message;
}

void Pose::locaEmitEvent(ParticleFilter::tLocalizationEvent event){
    loca->emit_event(event);
    //LOG_DATA << "Emit event: " << event;
}


Robot Pose::getRobotPoseWcs() {
    _robotWcs.pos = loca->get_position();

    _robotWcs.confidence = loca->get_confidence();

    // add ground truth data, if available and not too old.
    _robotWcs.GTtimestamp = poseBlackboard->gtTimestamp;
    if ((poseBlackboard->gtTimestamp > 0)
            && ((getTimestampMs() - poseBlackboard->gtTimestamp) < 5000)) {
        _robotWcs.GTpos = poseBlackboard->gtPosition;
    }

    _robotWcs.fallen = body->qns[IS_FALLEN] || body->qns[IS_STANDING_UP];

    return _robotWcs;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
