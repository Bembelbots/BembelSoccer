#pragma once

#include <unordered_map>
#include "jsvideosource.h"

class CameraV4L2;

class V6VideoSource : public JsVideoSource {
public:
    V6VideoSource(const int &cam_id);

    V6VideoSource(const V6VideoSource&) = delete;
    V6VideoSource& operator=(const V6VideoSource&) = delete;

    virtual ~V6VideoSource();
    virtual void fetchImage(CamImage &dst) override;
    virtual bool startCapturing() override;
    virtual int getParameter(CameraOption option) override;
    virtual void setParameter(CameraOption option, int value) override;
    virtual int getCamera() const override;
    virtual void setCamera(const int camera) override;
    virtual int getCameraResolution() const override;
    virtual bool setCameraResolution(const int res) override;

private:
    CameraV4L2 *cam;
    bool isTopCam;
    std::unordered_map<CameraDefinitions::CameraOption, CameraParameter> param;

    bool checkOpt(CameraOption option);
};

// vim: set ts=4 sw=4 sts=4 expandtab:
