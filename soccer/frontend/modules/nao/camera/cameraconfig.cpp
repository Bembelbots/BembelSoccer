#include "cameraconfig.h"
#include <framework/logger/logger.h>
#include <framework/util/assert.h>
#include <framework/thread/util.h>
#include <cmath>
#include <algorithm>
#include <representations/bembelbots/constants.h>

std::vector<std::vector<int>> CameraConfig::parameters(2, std::vector<int>(34, 0));
std::shared_ptr<NaoCameras> CameraConfig::cameras = nullptr;

void CameraConfig::setCameras(std::shared_ptr<NaoCameras> _cameras) {
    cameras = std::move(_cameras);
}

int CameraConfig::getParameter(const int camera,
                         CameraDefinitions::CameraOption option) {
    // return parameter stored in our database.
    // if not forced, this is the paramter stored
    // on the blackboard, which initially load from
    // the config file and then set.
    // there is no method to check if this is the
    // actual value set on the camera. but worked so
    // far...
    return parameters[camera][option];
}

void CameraConfig::setParameter(const int camera,
                          CameraDefinitions::CameraOption option, const int value,
                          const bool force) {
    
    if (!force && parameters[camera][option] == value) {
        return;
    }

    parameters[camera][option] = value;
    LOG_INFO << "Camera " << camera << " set " << option << " to " << value;

    if (!cameras->initialized()) {
        LOG_WARN << "ImageProvider: cameras not ready for parameter change";
        return;
    }

    if (camera == TOP_CAMERA) {
        cameras->top->setParameter(option, value);
    } else {
        cameras->bottom->setParameter(option, value);
    }

#if !V6
    sleep_for(200ms);
#endif
}

int CameraConfig::Exposure(const int camera) {
    return getParameter(camera, CameraDefinitions::Exposure);
}

void CameraConfig::Exposure(const int camera, const int value,
                      const bool force) {
    setParameter(camera, CameraDefinitions::Exposure, value, force);
}

int CameraConfig::Gain(const int camera) {
    return getParameter(camera, CameraDefinitions::Gain);
}

void CameraConfig::Gain(const int camera, const int value, const bool force) {
    setParameter(camera, CameraDefinitions::Gain, value, force);
}

int CameraConfig::Brightness(const int camera) {
    return getParameter(camera, CameraDefinitions::Brightness);
}

void CameraConfig::Brightness(const int camera, const int value,
                        const bool force) {
    setParameter(camera, CameraDefinitions::Brightness, value, force);
}

int CameraConfig::Contrast(const int camera) {
    return getParameter(camera, CameraDefinitions::Contrast);
}

void CameraConfig::Contrast(const int camera, const int value,
                      const bool force) {
    setParameter(camera, CameraDefinitions::Contrast, value, force);
}

int CameraConfig::Saturation(const int camera) {
    return getParameter(camera, CameraDefinitions::Saturation);
}

void CameraConfig::Saturation(const int camera, const int value,
                        const bool force) {
    setParameter(camera, CameraDefinitions::Saturation, value, force);
}

int CameraConfig::Sharpness(const int camera) {
    return getParameter(camera, CameraDefinitions::Sharpness);
}

void CameraConfig::Sharpness(const int camera, const int value,
                       const bool force) {
    setParameter(camera, CameraDefinitions::Sharpness, value, force);
}

int CameraConfig::WhiteBalance(const int camera) {
    return getParameter(camera, CameraDefinitions::WhiteBalance);
}

void CameraConfig::WhiteBalance(const int camera, const int value,
                          const bool force) {
    setParameter(camera, CameraDefinitions::WhiteBalance, value, force);
}

int CameraConfig::AutoExposure(const int camera) {
    return getParameter(camera, CameraDefinitions::AutoExposure);
}

void CameraConfig::AutoExposure(const int camera, const int value,
                          const bool force) {
    setParameter(camera, CameraDefinitions::AutoExposure, value, force);
}

int CameraConfig::AutoWhiteBalance(const int camera) {
    return getParameter(camera, CameraDefinitions::AutoWhiteBalance);
}

void CameraConfig::AutoWhiteBalance(const int camera, const int value,
                              const bool force) {
    setParameter(camera, CameraDefinitions::AutoWhiteBalance, value, force);
}
