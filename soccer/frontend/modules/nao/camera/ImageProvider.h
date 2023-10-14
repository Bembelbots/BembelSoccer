#pragma once

#include <vector>
#include <atomic>

#include "camera.h"
#include "image_buffer.h"
#include <nao/camera/cameradef.h>
#include <core/util/constants.h>

class CamImage;
class JsVideoSource;
class BodyState;

class ImageProvider {
public:
    ImageProvider(const size_t &size)
      : _bufferSize(size)
      , _topImages(size, TOP_CAMERA)
      , _bottomImages(size, BOTTOM_CAMERA) {}
    ~ImageProvider();

    void worker();
    void initCameras();
    void stop();
    bool isReady();
    void setParameter(const int &camera, CameraDefinitions::CameraOption option, const int &value);
    int getParameter(const int &camera, CameraDefinitions::CameraOption option);

    CamImage getImage(const int &camera);

private:
    int _bufferSize;
    static constexpr int _imageSize{camera::w * camera::h * camera::bpp};

    static void setCamPose(CamImage &img, camPose &cp);

    void openCamera(const int &cam);

    ImageBuffer _topImages;
    ImageBuffer _bottomImages;

    JsVideoSource *_topCam{nullptr};
    JsVideoSource *_bottomCam{nullptr};

    bool _capturing{true}; // run the worker!
    bool _finished{true};  // is the runner finish?
    bool _nullVideo{false};
};

// vim: set ts=4 sw=4 sts=4 expandtab:
