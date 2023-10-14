#pragma once
#include <framework/blackboard/blackboard.h>
#include <opencv2/imgproc.hpp>

class CameraCalibrationBlackboard : public Blackboard {
public:
    const std::string robot_name;

    explicit CameraCalibrationBlackboard(const std::string &robot_name, const std::string &blackboardName);
    ~CameraCalibrationBlackboard();

    bool loadConfig(ConfigFiles &) override;
    bool writeConfig(ConfigFiles &) override;

    MAKE_VAR(float, pitchCorrection);
    MAKE_VAR(float, principalPointX);
    MAKE_VAR(float, principalPointY);
    MAKE_VAR(float, focalLengthX);
    MAKE_VAR(float, focalLengthY);
    MAKE_VAR(float, distortion0);
    MAKE_VAR(float, distortion1);
    MAKE_VAR(float, distortion2);
    MAKE_VAR(float, distortion3);
    MAKE_VAR(float, distortion4);
    MAKE_VAR(float, yawCorrection);
    MAKE_VAR(float, fieldOfViewLeft);
    MAKE_VAR(float, fieldOfViewRight);
    MAKE_VAR(float, fieldOfViewTop);
    MAKE_VAR(float, fieldOfViewBottom);
    MAKE_VAR(cv::Mat, pixelAngles);
};
