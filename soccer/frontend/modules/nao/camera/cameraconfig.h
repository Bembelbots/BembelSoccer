#pragma once
#include <chrono>
#include <vector>
#include <stdint.h>
#include "cameradef.h"
#include "naocameras.h"

class CameraConfig {
public:
    static void setCameras(std::shared_ptr<NaoCameras>);

    static int Exposure(const int camera);
    static void Exposure(const int camera, const int value,
                         const bool force = false);

    static int Gain(const int camera);
    static void Gain(const int camera, const int value,
                     const bool force = false);

    static int Brightness(const int camera);
    static void Brightness(const int camera, const int value,
                           const bool force = false);

    static int Contrast(const int camera);
    static void Contrast(const int camera, const int value,
                         const bool force = false);

    static int Saturation(const int camera);
    static void Saturation(const int camera, const int value,
                           const bool force = false);

    static int Sharpness(const int camera);
    static void Sharpness(const int camera, const int value,
                          const bool force = false);

    static int WhiteBalance(const int camera);
    static void WhiteBalance(const int camera, const int value,
                             const bool force = false);

    static int AutoExposure(const int camera);
    static void AutoExposure(const int camera, const int value,
                             const bool force = false);

    static int AutoWhiteBalance(const int camera);
    static void AutoWhiteBalance(const int camera, const int value,
                                 const bool force = false);

protected:
    static std::vector<std::vector<int>> parameters;
    static std::shared_ptr<NaoCameras> cameras;

    static int getParameter(const int camera,
                            CameraDefinitions::CameraOption option);
    static void setParameter(const int camera,
                             CameraDefinitions::CameraOption option, const int value,
                             const bool force = false);
};
