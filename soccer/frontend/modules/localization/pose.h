#pragma once

#include "framework/rt/endpoints/input.h"
#include "gamecontrol_generated.h"
#include "particlefilter.h"

#include <framework/rt/module.h>
#include <framework/util/clock.h>

#include <representations/blackboards/settings.h>
#include <representations/motion/body_state.h>
#include <representations/vision/visiondefinitions.h>
#include <representations/worldmodel/definitions.h>
#include <representations/localization/locamessage.h>
#include <representations/flatbuffers/flatbuffers.h>

class PoseBlackboard;
class HypothesesGenerator;
class PlayingField;

class Pose : public rt::Module {
public:
    Pose();
    
    void connect(rt::Linker &) override;
    void process() override;
    void setup() override;

private:
    rt::Context<SettingsBlackboard> settings;
    rt::Context<PlayingField> playingfield;
    rt::Input<BodyState> body;
    rt::Input<bbapi::GamecontrolMessageT, rt::Listen> gamecontrol;
    rt::Input<VisionResultVec, rt::Require> visionResults;
    rt::Output<bbapi::LocalizationMessageT, rt::OutputFlag> locamessage;

    std::shared_ptr<PoseBlackboard> poseBlackboard;
    std::shared_ptr<HypothesesGenerator> locaHypoGenerator;
    
    //ParticleFilter
    ParticleFilter *loca;
    float sensor_angle_offset;

    Robot _robotWcs; ///< my belief about my own pose

    TimestampMs _beforestandUP; //pose only gets time of robot (approx.) 3 seconds before stand up

    //TOO !!! NOT CALCULATING IT
    /** @name handling special events @{ */
    bool _lastStateOfLostGroundContact; ///< state of lost ground
    bool _lastStateOfIsPenalized; ///< state of penalized
    /// number of measurements saying "penalized", to cope for being "accidentially" penalized
    int _isPenalizedCounter;
    /// number of measurements saying "lost ground contact", to be on the safe side
    int _lostGroundContactCounter;
    bool _wasFallen;
    void locaEmitEvent(ParticleFilter::tLocalizationEvent event);
    Robot getRobotPoseWcs();
};


// vim: set ts=4 sw=4 sts=4 expandtab:
