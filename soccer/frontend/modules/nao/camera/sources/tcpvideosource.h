#pragma once

#include <string>
#include "jsvideosource.h"

class TCPWrapper;

class TCPVideoSource : public JsVideoSource {
public:
    TCPVideoSource(const int &cam_id, std::string simulatorHost, bool docker);
    virtual ~TCPVideoSource();

    TCPVideoSource(TCPVideoSource&) = delete;
    TCPVideoSource(TCPVideoSource&&) = delete;
    TCPVideoSource& operator=(TCPVideoSource&&) = delete;

    virtual void fetchImage(CamImage &dst) override;
    virtual bool startCapturing() override;
    virtual int getParameter(CameraOption option) override;
    virtual void setParameter(CameraOption option, int value) override;
    virtual int getCamera() const override;
    virtual void setCamera(const int camera) override;
    virtual int getCameraResolution() const override;
    virtual bool setCameraResolution(const int res) override;

private:
    TCPWrapper *tcp;
    bool isTopCam;

    void connect();
    void disconnect();
};

// vim: set ts=4 sw=4 sts=4 expandtab:
