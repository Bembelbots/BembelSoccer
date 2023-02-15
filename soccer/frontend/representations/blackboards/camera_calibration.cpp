#include "camera_calibration.h"
#include <framework/logger/logger.h>
#include <string>
#include "../camera/camera.h"
#include <framework/util/configparser.h>
#include <framework/image/camimage.h>
#include <libbembelbots/config/calibration.h>
#include <iomanip>

CameraCalibrationBlackboard::CameraCalibrationBlackboard(const std::string &robot_name, const std::string &blackboardName)
  : Blackboard(blackboardName), robot_name(robot_name) {
    INIT_VAR_RW(pitchCorrection, 0, "correction value of pitch value");
    INIT_VAR_RW(yawCorrection, 0, "yaw offset");
    INIT_VAR(principalPointX, 0, "");
    INIT_VAR(principalPointY, 0, "");
    INIT_VAR(focalLengthX, 0, "");
    INIT_VAR(focalLengthY, 0, "");
    INIT_VAR(distortion0, 0, "");
    INIT_VAR(distortion1, 0, "");
    INIT_VAR(distortion2, 0, "");
    INIT_VAR(distortion3, 0, "");
    INIT_VAR(distortion4, 0, "");
    INIT_VAR(fieldOfViewLeft, 0, "");
    INIT_VAR(fieldOfViewRight, 0, "");
    INIT_VAR(fieldOfViewTop, 0, "");
    INIT_VAR(fieldOfViewBottom, 0, "");
    INIT_VAR(pixelAngles, cv::Mat(camera::w, camera::h, CV_32FC2), "");
}

CameraCalibrationBlackboard::~CameraCalibrationBlackboard() {
}

bool CameraCalibrationBlackboard::loadConfig(ConfigFiles &cfg) {
    READ_KEY_SUBSECTION(cfg.calib, getBlackboardName(), pitchCorrection, float);

    //const std::string name{tafel().nao.getHostname()};
    calibration::HeadCalibration::cam cal;

    if (calibration::head.count(robot_name)) {
        if (getBlackboardName() == "cameraCalibrationTop")
            cal = calibration::head.at(robot_name).topCam;
        else
            cal = calibration::head.at(robot_name).bottomCam;
    } else {
        // warn if no calibration available
        LOG_WARN << "Robot \xc2\xbb" << robot_name << "\xc2\xab has no calibration for \xc2\xbb" << getBlackboardName() << "\xc2\xab, using default values!";
    }

    principalPointX = cal.principalPoint[0];
    principalPointY = cal.principalPoint[1];
    focalLengthX = cal.focalLength[0];
    focalLengthY = cal.focalLength[1];
    distortion0 = cal.distortion[0];
    distortion1 = cal.distortion[1];
    distortion2 = cal.distortion[2];
    distortion3 = cal.distortion[3];
    distortion4 = cal.distortion[4];
    yawCorrection = cal.yawCorrection;

    fieldOfViewLeft = std::atan2(principalPointX, focalLengthX);
    fieldOfViewRight = std::atan2(camera::w - principalPointX, focalLengthX);
    fieldOfViewTop = std::atan2(principalPointY, focalLengthY);
    fieldOfViewBottom = std::atan2(camera::h - principalPointY, focalLengthY);
    
    pixelAngles = CamImage::calcPixelAngles(cal.principalPoint, cal.focalLength,{fieldOfViewLeft, fieldOfViewRight}, {fieldOfViewTop, fieldOfViewBottom}, cal.distortion);
    
    LOG_DEBUG << "Camera Calibration";
    LOG_DEBUG << this;
    return true;
}

bool CameraCalibrationBlackboard::writeConfig(ConfigFiles &cfg) {
    WRITE_KEY_SUBSECTION(cfg.calib, getBlackboardName(), pitchCorrection, float);
    return true;
}

std::ostream &operator<<(std::ostream &s, const CameraCalibrationBlackboard *rhs) {
    s << "current vision calibration blackboard content:\n";
    s << "  pitchCorrection:       " << std::setw(10) << rhs->pitchCorrection << "\n";
    s << "  yawCorrection:         " << std::setw(10) << rhs->pitchCorrection << "\n";
    s << "  principalPointX:       " << std::setw(10) << rhs->principalPointX << "\n";
    s << "  principalPointY:       " << std::setw(10) << rhs->principalPointY << "\n";
    s << "  focalLengthX:          " << std::setw(10) << rhs->focalLengthX << "\n";
    s << "  focalLengthY:          " << std::setw(10) << rhs->focalLengthY << "\n";
    s << "  distortion0:           " << std::setw(10) << rhs->distortion0 << "\n";
    s << "  distortion1:           " << std::setw(10) << rhs->distortion1 << "\n";
    s << "  distortion2:           " << std::setw(10) << rhs->distortion2 << "\n";
    s << "  distortion3:           " << std::setw(10) << rhs->distortion3 << "\n";
    s << "  distortion4:           " << std::setw(10) << rhs->distortion4;
    return s;
}
