
#include "imagethread.h"
#include <future>
#include "../camera/image_buffer.h"
#include "../camera/naocameras.h"
#include "../camera/cameraconfig.h"
#include <framework/util/clock.h>
#include <representations/blackboards/camera_calibration.h>

void ImageThread::setCamPose(CamImage &img, camPose &cp) {
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
    link(nao_info);
    link(image_provider);
    link(cmds);
}

void ImageThread::setup() {
    cmds.connect<SetPitchOffset, &ImageThread::onSetPitchOffset>(this);
    std::string name{nao_info->getHostname()};

    topCameraCalibration = std::make_unique<CameraCalibrationBlackboard>(name, "cameraCalibrationTop");
    bottomCameraCalibration = std::make_unique<CameraCalibrationBlackboard>(name, "cameraCalibrationBottom");
    cameraParameters = std::make_unique<CameraParametersBlackboard>();
    cameras = std::make_shared<NaoCameras>(settings->simulator);
    CameraConfig::setCameras(cameras);

    {
        // std::future destructor takes care of wait()

        // initializing cameras takes quite a while, so let's do it in parallel
        auto f1 = std::async(&NaoCameras::openCamera, cameras, TOP_CAMERA, settings->simulatorHost, settings->docker);
        auto f2 = std::async(&NaoCameras::openCamera, cameras, BOTTOM_CAMERA, settings->simulatorHost, settings->docker);
    }

    image_provider->top.initialize(500, TOP_CAMERA, settings->simulator);
    image_provider->bottom.initialize(500, BOTTOM_CAMERA, settings->simulator);

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
    if (!cameras->initialized()) {
        LOG_WARN_EVERY_N(10) << "cameras not initalized";
        return;
    }

    if(!once) {
        once = true;
        checkCameraParameters(true);
        return;
    }

    cmds.update();

    // updating camera parameters can take a while so lets do it in the background :D
    auto _ = std::async(&ImageThread::checkCameraParameters, this, false);

    // fetch images
    CamImage &tImg = image_provider->top.getCaptureBuffer();
    CamImage &bImg = image_provider->bottom.getCaptureBuffer();

    cameras->top->fetchImage(tImg); 
    cameras->bottom->fetchImage(bImg);

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

/*
int ImageProvider::getParameter(const int &camera, CameraOption option) {
    if (!_topCam || !_bottomCam) {
        LOG_WARN << "ImageProvider: cameras not ready for parameter change";
        return -1;
    }
    if (camera == TOP_CAMERA) {
        return _topCam->getParameter(option);
    }
    return _bottomCam->getParameter(option);
}

void ImageProvider::setParameter(const int &camera, CameraOption option, const int &value) {
    if (!_topCam || !_bottomCam) {
        LOG_WARN << "ImageProvider: cameras not ready for parameter change";
        return;
    }

    if (camera == TOP_CAMERA) {
        _topCam->setParameter(option, value);
    } else {
        _bottomCam->setParameter(option, value);
    }
}
*/
