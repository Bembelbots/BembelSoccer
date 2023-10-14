#pragma once
#include <framework/rt/module.h>
#include <representations/blackboards/settings.h>
#include <representations/blackboards/vision.h>
#include <representations/camera/image_provider.h>
#include <representations/vision/image.h>
#include <representations/nao/commands.h>
#include <mutex>

class VisionToolbox;
class PlayingField;

class Vision : public rt::Module {
public: 
    void connect(rt::Linker &) override;
    void process() override;
    void setup() override;
    void stop() override;
private:
    using BallFound = bool;
    using DetectResult = std::pair<BallFound, VisionResultVec>;

    rt::Context<SettingsBlackboard> settings;
    rt::Context<PlayingField> playingfield;
    rt::Context<ImageProvider, rt::Write> image_provider;
    
    rt::Output<VisionImageProcessed, rt::Event> processed;
    rt::Output<VisionResultVec> vision_results;

    rt::Command<NaoCommand> nao;

    VisionBlackboard board;
    float center_circle_radius;
    
    std::shared_ptr<VisionToolbox> top_toolbox;
    std::shared_ptr<VisionToolbox> bottom_toolbox;

    CamImage getImage(int cam);

    DetectResult processTopCam(CamImage img);
    DetectResult processBottomCam(CamImage img);

    DetectResult detect(CamImage &, VisionToolbox &toolbox);
    
    void autoCalibratePitchWithBall(float real_distance, CamImage &camera, VisionResultVec &vrs);

    //VisionResultVec complete(CamImage &image, VisionResultVec &results);
};
