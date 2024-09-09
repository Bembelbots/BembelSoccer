#include "v6videosource.h"

#include <chrono>
#include <exception>
#include <thread>
#include <filesystem>
#include <system_error>

#include <linux/v4l2-controls.h>
#include <representations/bembelbots/constants.h>
#include <representations/camera/camera.h>
#include <framework/logger/logger.h>
#include <framework/ipc/time.h>
#include <framework/util/clock.h>

#include "../CameraV4L2.hpp"

namespace fs = std::filesystem;
using namespace std::chrono_literals;
using std::this_thread::sleep_for;

V6VideoSource::V6VideoSource(const int &cam_id) {
    param = { // cppcheck-suppress useInitializationList
        {AutoExposure, {V4L2_CID_EXPOSURE_AUTO, 0, 1}},
        {AutoWhiteBalance, {V4L2_CID_AUTO_WHITE_BALANCE, 0, 1}},
        {Contrast, {V4L2_CID_CONTRAST, 0, 255}},
        {Exposure, {V4L2_CID_EXPOSURE_ABSOLUTE, 0, 1048575}},
        {Gain, {V4L2_CID_GAIN, 0, 1023}},
        {Hue, {V4L2_CID_HUE, -180, 180}},
        {Saturation, {V4L2_CID_SATURATION, 0, 255}},
        {Sharpness, {V4L2_CID_SHARPNESS, 0, 9}},
        {WhiteBalance, {V4L2_CID_WHITE_BALANCE_TEMPERATURE, 0, 6500}},
        {Brightness, {V4L2_CID_BRIGHTNESS, -255, 255}},
        {AutoHue, {V4L2_CID_HUE_AUTO, 0, 1}},
        {AutoFocus, {V4L2_CID_FOCUS_AUTO, 0, 1}},
        {Focus, {V4L2_CID_FOCUS_ABSOLUTE, 0, 250}},
    };


    isTopCam = (cam_id == TOP_CAMERA);
    fs::path dev{"/dev"};
    dev /= isTopCam ? "video-top" : "video-bottom";

    std::error_code ec;
    int c = 0;
    // `dev` must be a character device (or a symlink pointing to one)
    while (fs::status(dev).type() != fs::file_type::character) { 
        sleep_for(10ms);
        LOG_INFO_EVERY_N(100) << "Waiting for camera device: " << dev;
        if (++c > 2000)
            throw std::runtime_error("Error: could not open camera device " + dev.string());
    }

    cam = new CameraV4L2(dev, camera::h, camera::w, camera::fps);
    assert(cam);
    cam->init();
    assert(cam->isOpen());

    // clear bit 5 of register 0x5005
    // this makes AWB less greenish
    // https://github.com/HULKs/NaoV6/wiki/NaoV6-Camera
    uint16_t r{cam->readDeviceRegister(0x5005)};
    r &= ~(1<<5);
    cam->writeDeviceRegister(0x5005, r);

    cam->setParameter(V4L2_CID_FOCUS_AUTO, 0);
    cam->setParameter(V4L2_CID_FOCUS_ABSOLUTE, 0);
    cam->setParameter(V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_AUTO);
    cam->setParameter(V4L2_CID_HUE_AUTO, 1);
   
    if (isTopCam) {
        cam->setExtUnit(cam->HORIZONTAL_FLIP, 1);  // enable Horizontal Flip
        cam->setExtUnit(cam->VERTICAL_FLIP, 1);    // enable Vertical Flip
    }
}

V6VideoSource::~V6VideoSource() {
    delete cam;
}

void V6VideoSource::fetchImage(CamImage &dst) {
    dst.camera = getCamera();
    static constexpr int MAX_TRIES{3};
    int errCount{0};

    for (;;) {
        try {
            dst.timestamp = cam->getImage(dst.data);
            break;
        } catch (std::exception &e) {
            LOG_ERROR << __PRETTY_FUNCTION__ << " - failed to fetch image: " << e.what();
            if (errCount++ > MAX_TRIES)
                throw;
        }
    }

    if (dst.timestamp < 1) {
        dst.timestamp = getSystemTimestamp();
        // first few frames are usually missing timestamps, so silently ignore errors if framework is running for less than 5s
        LOG_ERROR_IF(getTimestampMs() > 5000) << "Camera timestamp was 0! Using current timestamp instead.";
    }
}

bool V6VideoSource::startCapturing() {
    cam->start();
    return true;
}


int V6VideoSource::getCamera() const {
    return !isTopCam;
}

void V6VideoSource::setCamera(const int camera) {
    // I don't know how to physically swap the cameras...
    WARN_NOT_IMPLEMENTED;
}

int V6VideoSource::getParameter(CameraOption option) {
    if (!checkOpt(option))
        return 0;
    return cam->getParameter(param[option].getControlId());
}

void V6VideoSource::setParameter(CameraOption option, int value) {
    if (!checkOpt(option))
        return;
    
    auto &p = param[option];
    if (option == AutoExposure)
        value = !value;
    cam->setParameter(p.getControlId(), p.clip(value));
}

bool V6VideoSource::setCameraResolution(const int /*res*/) {
    WARN_NOT_IMPLEMENTED;
    return true;
}

int V6VideoSource::getCameraResolution() const {
    WARN_NOT_IMPLEMENTED;
    return 0;
}


bool V6VideoSource::checkOpt(CameraOption option) {
    if (param.find(option) == param.end()) {
        LOG_WARN << "Camera parameter " << option << " not supported";
        return false;
    }
    return true;
}


// vim: set ts=4 sw=4 sts=4 expandtab:
