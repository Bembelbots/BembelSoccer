/**
 * @author Module owner: Bembelbots Frankfurt, holgerf
 *
 * $Id$
 *
 * This file contains...
 *
 * @file jsvideosource.h provide read images from camera or file stream
 * @file jsvideosource.cpp @see jsvideosource.h
 *
 */

#pragma once

// defines see jsvisionvars.h
#include <vector>
#include <array>
#include <boost/shared_ptr.hpp>
#include "../cameradef.h"
#include <framework/image/camimage.h>

using namespace CameraDefinitions;

/** inits reference to parent broker, to be called from jrlsoccer::preload, otherwise naodebug does not work
 * in principal inits static logger
 * @param parentProxy Sets naoqi parent broker
 * @param logmutex Sets logmutex for correct numbering in ALMemory
 */

class JsVideoSource {
public:
    virtual ~JsVideoSource() {}

    /**
     * private function to grab an image into
     * global fIplImageHeader variable.
     * this have to be reimplemented after RC,
     * since this is no good idea imho.
     */
    virtual void fetchImage(CamImage &dst) = 0;

    /**
     * start capturing
     * @return false on error
     */
    virtual bool startCapturing() {
        return true;
    }

    /**
      * get camera parameter
      * can handle v4l and naoqi camera parameter!
      */
    virtual int getParameter(CameraOption kid) = 0;

    /**
     * set camera parameter
     * this is a function for the V4L video grabber
     * and is not used in the naoqi video grabber version.
     * to enable this function, simply modify the USE_V4L define
     * @param id id of parameter to set. this is equal to the naoqi
     * parameters.
     * @param value value to set
     */
    virtual void setParameter(CameraOption kid, const int value) = 0;


    /**
     * select camera
     * @param camera number of camera (1:bottom, 0:top)
     */
    virtual void setCamera(const int /*camera*/) {};

    /**
     * get selected camera
     * @return The currently selected camera
     */
    virtual int getCamera() const {
        return 0;
    }

    /**
     * get camera resolution
     * @return current resoltion, 0 = kQQVGA, 1 = kQVGA, 2 = kVGA
     */
    virtual int getCameraResolution() const = 0;

    /**
     * set camera resolution
     * @param res resoltion, 0 = kQQVGA, 1 = kQVGA, 2 = kVGA
     * @return return true if switch was successful
     */
    virtual bool setCameraResolution(const int res) = 0;
};

class CameraParameter {
public:
    CameraParameter() = default;

    CameraParameter(int controlId, int min, int max)
        : _controlId(controlId), _min(min), _max(max) {}

    int getControlId() const {
        return _controlId;
    }

    int clip(int value) {
        return std::max(_min, std::min(value, _max));
    }

private:
    int _controlId{-1}, _min{-1}, _max{-1};
};

class JsVideoSourceV4LDataProxy;
class JsVideoSourceV4L: public JsVideoSource {
public:
    explicit JsVideoSourceV4L(const int &camera);

    JsVideoSourceV4L(const JsVideoSourceV4L&) = delete;
    JsVideoSourceV4L& operator=(const JsVideoSourceV4L&) = delete;

    ~JsVideoSourceV4L();
    void fetchImage(CamImage &dst) override;
    bool startCapturing() override;
    int getParameter(CameraOption option) override;
    void setParameter(CameraOption option, const int value) override;
    int getCamera() const override;
    void setCamera(const int camera) override;
    int getCameraResolution() const override;
    bool setCameraResolution(const int res) override;

private:
    JsVideoSourceV4LDataProxy *_data;
    std::array<CameraParameter, NUM_CAMERA_OPTIONS> parameters;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
