#pragma once

/*
 * This file contains hardware calibration for all
 * robots, which does not need to be re-calibrated
 * on a regular basis, such as OpenCV camera 
 * parameters or gyro gain on V6
*/

#include <array>
#include <string>
#include <unordered_map>

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

namespace calibration {

// body dependent calibration
struct HeadCalibration {
    // camera calibration (OpenCV paramters)
    struct cam {
        Eigen::Vector2f         principalPoint{320, 240};   // principal point
        Eigen::Vector2f         focalLength{630, 630};      // focal length
        cv::Vec<float, 5>       distortion{0, 0, 0, 0, 0};  // distortion
        float                   yawCorrection{0};           // yaw offset for poorly mounted cameras
    };

    cam topCam;     // OpenCV parameters for top camera
    cam bottomCam;  // OpenCV parameters for top camera
};


// body dependent calibration
struct BodyCalibration {
    Eigen::Vector3f gyroGain{1, 1, 1};  // gyro gain (X, Y, Z)
};


// map head serial -> calibration data
static const std::unordered_map<std::string, HeadCalibration> head{
};


// map body serial -> calibration data (currently V6 only)
static const std::unordered_map<std::string, BodyCalibration> body{
};


};


// vim: set ts=4 sw=4 sts=4 expandtab:
