#include "imagethread.h"
#include <future>
#include <cstdlib>
#include "../camera/image_buffer.h"
#include "../camera/naocameras.h"
#include "../camera/cameraconfig.h"
#include <framework/util/clock.h>
#include <representations/blackboards/camera_calibration.h>
#include <stdexcept>
#include <thread>

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

void ImageThread::setCamPose(CamImage &img, CamPose &cp) {
    CameraCalibrationBlackboard *cal;

    // choose correct camPose & calibration blackboard
    if (img.camera == TOP_CAMERA)
        cal = topCameraCalibration.get();
    else
        cal = bottomCameraCalibration.get();

    // apply pitch correction & set transformation
    cp.r[1] += cal->pitchCorrection;
    cp.r[2] += cal->yawCorrection;
    img.setTransform(cp);

    // set calibrationfetchImage
    img.setCalibration(cal->principalPointX, cal->principalPointY, cal->pixelAngles);
}

void ImageThread::connect(rt::Linker &link) {
    link.name = "ImageThread";
    link(settings);
    link(nao_states);
    link(game_state);
    link(nao_info);
    link(image_provider);
    link(camPose);
    link(cmds);
}

void ImageThread::setup() {
    cmds.connect<SetPitchOffset, &ImageThread::onSetPitchOffset>(this);
    std::string name{nao_info->getHostname()};

    topCameraCalibration = std::make_unique<CameraCalibrationBlackboard>(name, "cameraCalibrationTop");
    bottomCameraCalibration = std::make_unique<CameraCalibrationBlackboard>(name, "cameraCalibrationBottom");
    cameraParameters = std::make_unique<CameraParametersBlackboard>();

    image_provider->top.initialize(150, TOP_CAMERA, settings->simulator);
    image_provider->bottom.initialize(150, BOTTOM_CAMERA, settings->simulator);

    CameraConfig::setCameras([=](){
        return cameras.get();
    });

    resetCameras(false);

    LOG_INFO << "ImageProvider: started the cameras";
}

void ImageThread::checkCameraParameters(bool force = false) {
    if(!cameras->initialized()) {
        return;
    }

    for (int camera: {BOTTOM_CAMERA, TOP_CAMERA}) {
        CameraConfig::AutoExposure(camera, cameraParameters->autoExposure, force);
        CameraConfig::AutoWhiteBalance(camera, cameraParameters->autoWhiteBalancing, force);
        CameraConfig::Gain(camera, cameraParameters->gain, force);
        CameraConfig::Contrast(camera, cameraParameters->contrast, force);
        CameraConfig::Saturation(camera, cameraParameters->saturation, force);
        CameraConfig::Sharpness(camera, cameraParameters->sharpness, force);

        if (!cameraParameters->autoWhiteBalancing) {
            CameraConfig::WhiteBalance(camera, cameraParameters->whiteBalance, force);
        }

        if (camera == BOTTOM_CAMERA) {
            CameraConfig::Exposure(camera, cameraParameters->exposureBottom, force);
            CameraConfig::Brightness(camera, cameraParameters->brightnessBottom, force);
        } else {
            CameraConfig::Exposure(camera, cameraParameters->exposureTop, force);
            CameraConfig::Brightness(camera, cameraParameters->brightnessTop, force);
        }
    }
}

void ImageThread::process() {
    static bool once = false;

    if (!cameras->initialized())
        initCameras();
    
    if (!once) {
        once = true;
        checkCameraParameters(true);
        return;
    }

    // updating camera parameters can take a while so lets do it in the background :D
    auto _ = std::async(&ImageThread::checkCameraParameters, this, false);

    // fetch images
    CamImage &tImg = image_provider->top.getCaptureBuffer();
    CamImage &bImg = image_provider->bottom.getCaptureBuffer();

    bool has_image = false;
    int reset_counter = 0;

    while(!has_image) {
        if(reset_counter >= 3) {
            throw std::runtime_error("Camera reset failed after too many tries");
        }
        try {
            cameras->top->fetchImage(tImg); 
            cameras->bottom->fetchImage(bImg);
            has_image = true;
        } catch (...) {
            resetCameras();
            sleep_for(1s);
            initCameras();
            once = false;
        }
        reset_counter++;
    }

    // no body states available, wait for next one
    if (bs_dq.empty())
        nao_states.waitWhileEmpty();

    // append bodystates
    for (auto &s : nao_states.fetch()) {
        bs_dq.emplace_back(std::move(s));
    }

    NaoState topState, bottomState;
    topState.lola_timestamp = bottomState.lola_timestamp = INT64_MIN; // ensure any state will be better than this
    int64_t tdiff{INT64_MAX}, bdiff{INT64_MAX};

    // find best body state matches
    for (auto &i : bs_dq) {
        auto d = std::abs(tImg.timestamp - i.lola_timestamp);
        if (d < tdiff) {
            tdiff = d;
            topState = i;
        }

        d = std::abs(bImg.timestamp - i.lola_timestamp);
        if (d < bdiff) {
            bdiff = d;
            bottomState = i;
        }
    }

    // store images in ringbuffer
    setCamPose(tImg, topState.tCamPose);
    image_provider->top.releaseCaptureBuffer();
    setCamPose(bImg, bottomState.bCamPose);
    image_provider->bottom.releaseCaptureBuffer();

    bbapi::CamPoseMessageT cpm;
    cpm.top = topState.tCamPose;
    cpm.bottom = bottomState.bCamPose;
    camPose.emit(cpm);

    // prune queue
    while (bs_dq.front().lola_timestamp < std::min(topState.lola_timestamp, bottomState.lola_timestamp))
        bs_dq.pop_front();

    // print error if timestamps for BodyState and image are more that one LoLa cycle out of sync
    if (getTimestampMs() > 5000) { 
        constexpr auto limit{1.5f * CONST::lola_cycle_ms * 1000};
        LOG_ERROR_IF(std::abs(tImg.timestamp - topState.lola_timestamp) > limit)
                << "ImageProvider: BodyState for top camera out of sync!";
        LOG_ERROR_IF(std::abs(bImg.timestamp - bottomState.lola_timestamp) > limit)
                << "ImageProvider: BodyState for bottom camera out of sync!";
    }


    // TODO: GIANT HACK, PLEASE FIX
    static int realTopBrightness = -1;
    static int realContrast = -1;
    static int realSaturation = -1;
    static bool hasStandbyParameters = false;
    if (game_state->gameStateReal == bbapi::GameState::STANDBY && !hasStandbyParameters) {
        realTopBrightness = cameraParameters->brightnessTop;
        realContrast = cameraParameters->contrast;
        realSaturation = cameraParameters->saturation;
        LOG_DEBUG << "Setting standby camera parameters";
        cameraParameters->brightnessTop = 10;
        cameraParameters->contrast = 20;
        cameraParameters->saturation = 30;
        hasStandbyParameters = true;
    }
    if (game_state->gameStateReal != bbapi::GameState::STANDBY && hasStandbyParameters) {
        LOG_DEBUG << "Unsetting standby camera parameters";
        cameraParameters->brightnessTop = realTopBrightness;
        cameraParameters->contrast = realContrast;
        cameraParameters->saturation = realSaturation;
        hasStandbyParameters = false;
    }
}

void ImageThread::onSetPitchOffset(SetPitchOffset& data) {
    jsassert(data.camera == TOP_CAMERA || data.camera == BOTTOM_CAMERA);

    if(data.camera == TOP_CAMERA) {
        topCameraCalibration->pitchCorrection += data.offset;
    } else {
        bottomCameraCalibration->pitchCorrection += data.offset;
    }
}

void ImageThread::stop() {
    LOG_INFO << "image thread stopped";
}

void ImageThread::initCameras() {
    // initializing cameras takes quite a while, so let's do it in parallel
    auto f1 = std::async(&NaoCameras::openCamera, cameras, TOP_CAMERA, settings->simulatorHost, settings->docker);
    auto f2 = std::async(&NaoCameras::openCamera, cameras, BOTTOM_CAMERA, settings->simulatorHost, settings->docker);
    // std::future destructor takes care of wait()
}

void ImageThread::resetCameras(bool do_say) {
    LOG_WARN << "resetting cameras";
    try {
        cameras.reset();
        if (!settings->simulator)
            std::system("/opt/aldebaran/libexec/reset-cameras.sh toggle");
        cameras = std::make_shared<NaoCameras>(settings->simulator);
    } catch (...) {
        // do nothing
    }
    
    if(do_say) {
        LOG_SAY << "camera reset";
    }
}
