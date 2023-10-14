/**
 * @author Module owner: Bembelbots Frankfurt, holgerf
 *
 * $Id$
 *
 */

#include "jsvideosource.h"
#if V6
#include <linux/v4l2-controls.h>
#endif

#include "../NaoCamera.h"
#include <representations/camera/camera.h>

#include <framework/logger/logger.h>

using namespace std;


#if !V6
// **************                             **************
// ************** VV   VV  44        LL       **************
// **************  VV VV   44  44    LL       **************
// **************   VVV    44444444  LL       **************
// **************    V         44    LLLLLLL  **************
// **************                             **************
// Video for Linux (V4L) implementation


class JsVideoSourceV4LDataProxy {
public:
    NaoCamera::Camera *_camera; // top camera
    v4l2_buffer *_buffer; // v4l
    int _currentCameraId; // currently used camera
};


JsVideoSourceV4L::JsVideoSourceV4L(const int &camera) {
    _data = new JsVideoSourceV4LDataProxy;
    NaoCamera::Settings set(
        120,  //settings().robot.calibration.bottomCamera.exposure,
        40, //settings().robot.calibration.bottomCamera.brightness,
        60, //settings().robot.calibration.bottomCamera.gain,
        70, //settings().robot.calibration.bottomCamera.contrast,
        120, //settings().robot.calibration.bottomCamera.saturation,
        7, //settings().robot.calibration.bottomCamera.sharpness,
        6500  //settings().robot.calibration.bottomCamera.whiteBalance
    );

    // init cameras
    _data->_camera = new NaoCamera::Camera(set, camera);

    // set currently used camera
    _data->_currentCameraId = camera;
    _data->_buffer = new v4l2_buffer;

    parameters[AutoExposure] = CameraParameter(V4L2_CID_EXPOSURE_AUTO, 0, 3);
    parameters[AutoExposureAlgorithm] = CameraParameter(
                                            V4L2_CID_EXPOSURE_ALGORITHM, 0, 3);
    parameters[BacklightCompensation] = CameraParameter(
                                            V4L2_CID_BACKLIGHT_COMPENSATION, 0, 4);
    parameters[AutoWhiteBalance] = CameraParameter(V4L2_CID_AUTO_WHITE_BALANCE, 0,
                                   1);
    parameters[Contrast] = CameraParameter(V4L2_CID_CONTRAST, 16, 64);
    parameters[Exposure] = CameraParameter(V4L2_CID_EXPOSURE, 0, 1000);
    parameters[FadeToBlack] = CameraParameter(V4L2_MT9M114_FADE_TO_BLACK, 0, 1);
    parameters[Gain] = CameraParameter(V4L2_CID_GAIN, 0, 255);
    parameters[Hue] = CameraParameter(V4L2_CID_HUE, -22, 22);
    parameters[Saturation] = CameraParameter(V4L2_CID_SATURATION, 0, 255);
    parameters[Sharpness] = CameraParameter(V4L2_CID_SHARPNESS, -7, 7);
    parameters[WhiteBalance] = CameraParameter(V4L2_CID_WHITE_BALANCE_TEMPERATURE,
                               2700, 6500);
    parameters[Gamma] = CameraParameter(V4L2_CID_GAMMA, 100, 280);
    parameters[PowerLineFrequency] = CameraParameter(V4L2_CID_POWER_LINE_FREQUENCY,
                                     1, 2);
    parameters[TargetAverageLuma] = CameraParameter(
                                        V4L2_MT9M114_AE_TARGET_AVERAGE_LUMA, 0, 255);
    parameters[TargetAverageLumaDark] = CameraParameter(
                                            V4L2_MT9M114_AE_TARGET_AVERAGE_LUMA_DARK, 0, 255);
    parameters[TargetGain] = CameraParameter(V4L2_MT9M114_AE_TARGET_GAIN, 0,
                             65535);
    parameters[MinGain] = CameraParameter(V4L2_MT9M114_AE_MIN_VIRT_GAIN, 0, 65535);
    parameters[MaxGain] = CameraParameter(V4L2_MT9M114_AE_MAX_VIRT_GAIN, 0, 65535);
}

JsVideoSourceV4L::~JsVideoSourceV4L() {
    //jsassert(_data);
    //jsassert(_data->_camera);
    _data->_camera->stop_capturing();
    delete _data->_camera;
    delete _data->_buffer;
    delete _data;
}

void JsVideoSourceV4L::fetchImage(CamImage &dst) {
    auto &cam = *_data->_camera;
    auto buf = _data->_buffer;

    cam.get_image(buf);
    memcpy(dst.data, cam.start(buf), camera::w*camera::h*camera::bpp);
    dst.timestamp = (buf->timestamp.tv_sec * 1000) +
                    (buf->timestamp.tv_usec / 1000);
    dst.camera = _data->_currentCameraId;
    cam.release_image(buf);
}


bool JsVideoSourceV4L::startCapturing() {
    //jsassert(_data);
    //jsassert(_data->_camera);
    _data->_camera->start_capturing();

    return true;
}


int JsVideoSourceV4L::getCamera() const {
    return _data->_currentCameraId;
}

void JsVideoSourceV4L::setCamera(const int camera) {
    _data->_currentCameraId = camera;
}

int JsVideoSourceV4L::getParameter(CameraOption option) {
    CameraParameter parameter = parameters[option];
    return _data->_camera->getControl(parameter.getControlId());
}

void JsVideoSourceV4L::setParameter(CameraOption option, const int value) {
    CameraParameter parameter = parameters[option];
    _data->_camera->setControl(parameter.getControlId(), parameter.clip(value));
}

bool JsVideoSourceV4L::setCameraResolution(const int /*res*/) {
    WARN_NOT_IMPLEMENTED;
    return true;
}

int JsVideoSourceV4L::getCameraResolution() const {
    WARN_NOT_IMPLEMENTED;
    return 0;
}
#endif

// EOF
// vim: set ts=4 sw=4 sts=4 expandtab:
