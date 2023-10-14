#include "camimage.h"
#include <framework/util/assert.h>
#include <cmath>
#include <algorithm>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include <framework/logger/logger.h>
#include <framework/math/rotation_matrices.h>
#include <cstring>

void CamImage::_init() {
    // what are good values?
    principalPointX = 319.5f;
    principalPointY = 239.5f;

    _eulerMatrixCached = false;
}

void CamImage::setTransform(const camPose &eulers) {
    if (_eulers != eulers) {
        _eulers = eulers;
        _eulerMatrixCached = false;
    }
}

void CamImage::setCalibration(const float &pPointX, const float &pPointY, const float &fLenX, const float &fLenY,
        const float &foVL, const float &foVR, const float &foVT, const float &foVB, const float &distK1,
        const float &distK2, const float &distK3, const float &distP1, const float &distP2) {
    principalPointX = pPointX;
    principalPointY = pPointY;

    focalLengthX = fLenX;
    focalLengthY = fLenY;

    fieldOfViewLeft = foVL;
    fieldOfViewRight = foVR;
    fieldOfViewTop = foVT;
    fieldOfViewBottom = foVB;

    distCoeffs = {distK1, distK2, distP1, distP2, distK3};
}

void CamImage::setCalibration(const float &pPointX, const float &pPointY, cv::Mat pixAng) {
    principalPointX = pPointX;
    principalPointY = pPointY;
    pixelAngles = pixAng;
}

Coord CamImage::getRcsPosition(size_t x, size_t y) {
    Eigen::Vector2f angle;

    if (!pixelAngles) {
        LOG_WARN << __PRETTY_FUNCTION__ << ": 'pixelAngles' empty, recalculating...";
        pixelAngles = calcPixelAngles({principalPointX, principalPointY},
                {focalLengthX, focalLengthY},
                {fieldOfViewLeft, fieldOfViewRight},
                {fieldOfViewTop, fieldOfViewBottom},
                distCoeffs);
    }

    jsassert(x < width);
    jsassert(y < height);

    auto point = pixelAngles->at<cv::Point2f>(x, y);

    angle[0] = point.x;
    angle[1] = point.y;

    return getRcsTranslationFromAngles(angle);
}

void CamImage::_generateEulerMatrix() {
    jsassert(3 == _eulers.r.size());

    _cachedEulerMatrix = RotMat::rotateRPY(_eulers.r);

    _eulerMatrixCached = true;
}

/**
* needed here for dynamic ScanLines
*/
Eigen::Vector2f CamImage::getAngleFromPosition(const int &x, const int &y) const {
    Eigen::Vector2f angle;

    angle[0] = (x <= principalPointX)
                       ? fieldOfViewLeft * ((principalPointX - static_cast<float>(x)) / principalPointX)
                       : -fieldOfViewRight * ((static_cast<float>(x) - principalPointX) / (width - principalPointX));
    angle[1] = (y <= principalPointY)
                       ? -fieldOfViewTop * ((principalPointY - static_cast<float>(y)) / principalPointY)
                       : fieldOfViewBottom * ((static_cast<float>(y) - principalPointY) / (height - principalPointY));

    // angles are counterclockwise!!!
    return angle;
}

Coord CamImage::getRcsTranslationFromAngles(const Eigen::Vector2f &camAngles) {
    jsassert(3 == _eulers.v.size());
    jsassert(2 == camAngles.size());
    Eigen::Vector4f view{1.f, 0.f, 0.f, 1.f};

    // calculate vector in viewing direction (trough pixel)
    view[0] = 1;
    view[1] = sinf(camAngles[0]);
    view[2] = -sinf(camAngles[1]);

    if (!_eulerMatrixCached) {
        _generateEulerMatrix();
    }
    view = _cachedEulerMatrix * view;

    if (0.f < view[2]) {
        LOG_ERROR << "This pixel is in the sky! " << view[2];
        return Coord(1000000.f, 1000000.f); //return Coord(std::nanf(""), std::nanf(""));
    }
    float a = _eulers.v[2] / -(view[2]);

    return Coord(_eulers.v[0] + a * view[0], _eulers.v[1] + a * view[1]);
}

void CamImage::write(std::ofstream &file) const {
    file.write(reinterpret_cast<char *>(data), width * height * channels);

    for (int i = 0; i < _eulers.v.size(); i++) {
        file.write(reinterpret_cast<const char *>(&_eulers.v[i]), sizeof(float));
    }
    for (int i = 0; i < _eulers.r.size(); i++) {
        file.write(reinterpret_cast<const char *>(&_eulers.r[i]), sizeof(float));
    }

    file.write(reinterpret_cast<const char *>(&principalPointX), sizeof(float)); // cppcheck-suppress invalidPointerCast
    file.write(reinterpret_cast<const char *>(&principalPointY), sizeof(float)); // cppcheck-suppress invalidPointerCast
}

CamImage CamImage::deepCopy() const {
    CamImage other(*this);
    uint8_t *d = new u_int8_t[height * width * channels];
    std::memcpy(d, data, height * width * channels);
    other.setData(d);
    return other;
}

cv::Mat CamImage::getPixelAngles() const {
    jsassert(pixelAngles);
    return *pixelAngles;
}

cv::Mat CamImage::calcPixelAngles(const Eigen::Vector2f &pPoint, const Eigen::Vector2f &fLength,
        const Eigen::Vector2f &fovLR, const Eigen::Vector2f &fovTB, const cv::Vec<float, 5> &dist) {
    cv::Mat pxAngles(camera::w, camera::h, CV_32FC2);
    cv::Mat allPixels(camera::w, camera::h, CV_32FC2);
    cv::Matx33f cam;
    cam << fLength[0], 0.f, pPoint[0], 0.f, fLength[1], pPoint[1], 0.f, 0.f, 1.f;

    for (int i = 0; i < camera::h; i++) {
        for (int j = 0; j < camera::w; j++) {
            allPixels.at<cv::Point2f>(j, i) = cv::Point2f(j, i);
        }
    }
    cv::Mat pixelLocations(camera::w * camera::h, 1, CV_32FC2);
    cv::undistortPoints(allPixels.reshape(0, camera::w * camera::h), pixelLocations, cam, dist);
    pixelLocations = pixelLocations.reshape(0, camera::w);

    for (int i = 0; i < camera::h; i++) {
        for (int j = 0; j < camera::w; j++) {
            pixelLocations.at<cv::Point2f>(j, i).x = pixelLocations.at<cv::Point2f>(j, i).x * fLength[0] + pPoint[0];
            pixelLocations.at<cv::Point2f>(j, i).y = pixelLocations.at<cv::Point2f>(j, i).y * fLength[1] + pPoint[1];

            pxAngles.at<cv::Point2f>(j, i).x =
                    (pixelLocations.at<cv::Point2f>(j, i).x <= pPoint[0])
                            ? fovLR[0] * ((pPoint[0] - pixelLocations.at<cv::Point2f>(j, i).x) / pPoint[0])
                            : -fovLR[1] *
                                      ((pixelLocations.at<cv::Point2f>(j, i).x - pPoint[0]) / (camera::w - pPoint[0]));
            pxAngles.at<cv::Point2f>(j, i).y =
                    (pixelLocations.at<cv::Point2f>(j, i).y <= pPoint[1])
                            ? -fovTB[0] * ((pPoint[1] - pixelLocations.at<cv::Point2f>(j, i).y) / pPoint[1])
                            : fovTB[1] *
                                      ((pixelLocations.at<cv::Point2f>(j, i).y - pPoint[1]) / (camera::h - pPoint[1]));
        }
    }

    return pxAngles;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
