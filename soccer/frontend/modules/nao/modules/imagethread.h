#pragma once

#include <representations/camera/camera.h>
#include <framework/rt/module.h>
#include <representations/bembelbots/constants.h>
#include <representations/blackboards/settings.h>
#include <representations/blackboards/camera_calibration.h>
#include <representations/blackboards/camera_parameters.h>
#include <representations/bembelbots/nao_info.h>
#include <representations/camera/image_provider.h>
#include <representations/nao/commands.h>
#include "../naostate.h"
#include <deque>

//class DoCameraParameter{};
class NaoCameras;

class ImageThread : public rt::Module {
public:

    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;
    void stop() override;

private:
/*
    // TODO: get set camera parameter via commands
    rt::Command<DoCameraParameter, rt::Handle> cmds;
*/
    rt::Context<SettingsBlackboard> settings;
    rt::Input<NaoState, rt::Snoop> nao_states;
    rt::Context<NaoInfo> nao_info;
    rt::Context<ImageProvider, rt::Write> image_provider;
    rt::Command<NaoCommand, rt::Handle> cmds;

    std::shared_ptr<NaoCameras> cameras;

    std::unique_ptr<CameraCalibrationBlackboard> topCameraCalibration;
    std::unique_ptr<CameraCalibrationBlackboard> bottomCameraCalibration;
    std::unique_ptr<CameraParametersBlackboard> cameraParameters;

    std::deque<NaoState> bs_dq;

    void setCamPose(CamImage &img, camPose &cp);
    void checkCameraParameters(bool force);
    void onSetPitchOffset(SetPitchOffset &);
};

// vim: set ts=4 sw=4 sts=4 expandtab:
