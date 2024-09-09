#pragma once
#include <memory>
#include <framework/image/camimage.h>
#include <framework/util/clock.h>
#include <framework/datastructures/mpsc_storage.h>
#include <opencv2/imgcodecs.hpp>
#include <vector>
#include "framework/image/rgb.h"
#include "visiondefinitions.h"
#include "visioncontext.h"

struct VisionImageBase {
    CamImage image;

    VisionImageBase() : image() {}

    VisionImageBase(CamImage &_image) : image(_image) {}

    CamImage &operator*() { return image; }
};

struct VisionImage : public VisionImageBase {
    int id;
    VisionContext context;
    MPSCStorage<std::vector<VisionResult>> results;

    VisionImage() = default;

    VisionImage(CamImage &_image) : VisionImageBase(_image) { image.lock(ImgLock::VISION); }

    ~VisionImage() { image.unlock(ImgLock::VISION); }
};

struct VisionImageProcessed {
    using jpeg_t = std::vector<u_char>;
    using vr_t = std::vector<VisionResult>;

    size_t width;
    size_t height;
    int64_t timestamp;

    int camera{-1};
    float principalPointX{0};
    float principalPointY{0};

    float fieldOfViewLeft{0}, fieldOfViewRight{0};
    float fieldOfViewTop{0}, fieldOfViewBottom{0};

    float focalLengthX{0}, focalLengthY{0};

    CamPose _eulers;

    cv::Mat3f cameraMatrix;
    cv::Vec<float, 5> distCoeffs;

    std::shared_ptr<vr_t> results;
    std::shared_ptr<jpeg_t> jpeg;

    VisionImageProcessed() = default;

    explicit VisionImageProcessed(CamImage &img, vr_t vr)
      : width(img.width)
      , height(img.height)
      , timestamp(img.timestamp)
      , camera(img.camera)
      , principalPointX(img.principalPointX)
      , principalPointY(img.principalPointY)
      , fieldOfViewLeft(img.fieldOfViewLeft)
      , fieldOfViewRight(img.fieldOfViewRight)
      , fieldOfViewTop(img.fieldOfViewTop)
      , fieldOfViewBottom(img.fieldOfViewBottom)
      , focalLengthX(img.focalLengthX)
      , focalLengthY(img.focalLengthY)
      , _eulers(img._eulers)
      , cameraMatrix(img.cameraMatrix)
      , distCoeffs(img.distCoeffs) {
        std::vector<int> parms{cv::IMWRITE_JPEG_QUALITY, 80};
        jpeg = std::make_shared<jpeg_t>();
        cv::imencode(".jpg", img.toRGB().mat(), *jpeg, parms);

        results = std::make_shared<vr_t>();
        *results = vr;
    }
};
