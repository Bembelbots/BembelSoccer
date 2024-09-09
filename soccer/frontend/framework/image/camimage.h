#pragma once

#include <optional>
#include <representations/camera/cam_pose_struct.h>
#include <framework/math/coord.h>
#include <representations/camera/camera.h>
#include <Eigen/Dense>

#include "yuv422.h"

class CamImage : public YuvImage {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    int camera{-1};
    float principalPointX{0};
    float principalPointY{0};

    float fieldOfViewLeft{0}, fieldOfViewRight{0};
    float fieldOfViewTop{0}, fieldOfViewBottom{0};

    float focalLengthX{0}, focalLengthY{0};

    CamPose _eulers;

    cv::Mat3f cameraMatrix;
    cv::Vec<float, 5> distCoeffs;

// camera field of view
// got those from the nao documenatation. those are only used as a fallback because otherwise not accurate enough.
    // (pi / 180) * 56.3
    static constexpr float VIEW_HORIZONTAL = 0.982620f;
    // (pi / 180) * 43.7
    static constexpr float VIEW_VERTICAL = 0.762709f;

    // default constructor
    CamImage() : YuvImage() { _init(); };

    // constructor with given size and camera to use
    // image will be simply inited, but no data available.
    // \todo: access will fail, init raw image with black
    //        pixels.
    CamImage(uint32_t width, uint32_t height, int camera) : YuvImage(width, height), camera(camera) { _init(); };

    // constructor with image data to set.
    // WARNING: this class does not take care about memory usage.
    // so be sure to cleanup you data after not needing this class
    // anymore!
    CamImage(uint8_t *data, uint32_t width, uint32_t height, int camera)
      : YuvImage(data, width, height), camera(camera) {
        _init();
    };

    // get camera transformation.
    // this is the orientation (translation and rotation) from the head joint
    // related to the point between the ankles of the bot.
    // \todo: more documenatation.
    void setTransform(const CamPose &eulers);

    void setCalibration(const float &pPointX, const float &pPointY, const float &fLenX, const float &fLenY,
            const float &foVL = VIEW_HORIZONTAL / 2.f, const float &foVR = VIEW_HORIZONTAL / 2.f,
            const float &foVT = VIEW_VERTICAL / 2.f, const float &foVB = VIEW_VERTICAL / 2.f, const float &distK1 = 0.f,
            const float &distK2 = 0.f, const float &distK3 = 0.f, const float &distP1 = 0.f, const float &distP2 = 0.f);

    void setCalibration(const float &pPointX, const float &pPointY, cv::Mat pixAng);

    // returns an RCS position in meters for a given point in the image.
    // for this, a camera transormation matrix must be included in this image.
    Coord getRcsPosition(size_t x, size_t y);

    // write image to a given filename destination.
    virtual void write(std::ofstream &file) const;

    Eigen::Vector2f getAngleFromPosition(const int &x, const int &y) const;
    Eigen::Vector2f getAngleFromPosition(const cv::Point2f &p);

    Coord getRcsTranslationFromAngles(const Eigen::Vector2f &camAngles);

    // creates a full copy of *this, including copying the underlying data
    // WARNING: you need to call delete[] on the resulting images data pointer!
    CamImage deepCopy() const;

    cv::Mat getPixelAngles() const;

    static cv::Mat calcPixelAngles(const Eigen::Vector2f &pPoint, const Eigen::Vector2f &fLength,
            const Eigen::Vector2f &fovLR, const Eigen::Vector2f &fovTB, const cv::Vec<float, 5> &dist);

protected:
    std::optional<cv::Mat> pixelAngles;

    bool _eulerMatrixCached;

    // fixes this weird issue that should have gone away with c++17 (https://eigen.tuxfamily.org/dox/group__TopicUnalignedArrayAssert.html):
    Eigen::Matrix<float, 4, 4, Eigen::DontAlign> _cachedEulerMatrix;

    void _generateEulerMatrix();
    void _init();
};

// vim: set ts=4 sw=4 sts=4 expandtab:
