#include "camera_parameters.h"
#include <iomanip>
#include <framework/util/configparser.h>
#include <framework/logger/logger.h>

const std::string cfgCameraParametersSection = "camera";

CameraParametersBlackboard::CameraParametersBlackboard() : Blackboard("CameraParameter") {
    INIT_VAR_RW(exposureTop, 0, "");
    INIT_VAR_RW(exposureBottom, 0, "");
    INIT_VAR_RW(brightnessTop, 0, "");
    INIT_VAR_RW(brightnessBottom, 0, "");
    INIT_VAR_RW(gain, 0, "");
    INIT_VAR_RW(contrast, 0, "");
    INIT_VAR_RW(saturation, 0, "");
    INIT_VAR_RW(sharpness, 0, "");
    INIT_VAR_RW(whiteBalance, 0, "");
    INIT_VAR_RW(autoExposure, 0, "");
    INIT_VAR_RW(autoWhiteBalancing, 0, "");
}

CameraParametersBlackboard::~CameraParametersBlackboard() {
}

bool CameraParametersBlackboard::loadConfig(ConfigFiles &cfg) {
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, exposureTop, int);
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, exposureBottom, int);
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, gain, int);
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, contrast, int);
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, saturation, int);
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, sharpness, int);
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, whiteBalance, int);
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, autoExposure, int);
    READ_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, autoWhiteBalancing, int);
    
    // don't fail when bot has old calibration file without these values
    READ_KEY_SUBSECTION_TRY(cfg.calib, cfgCameraParametersSection, brightnessTop, int);
    READ_KEY_SUBSECTION_TRY(cfg.calib, cfgCameraParametersSection, brightnessBottom, int);

    return true;
}

bool CameraParametersBlackboard::writeConfig(ConfigFiles &cfg) {
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, exposureTop, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, exposureBottom, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, brightnessTop, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, brightnessBottom, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, gain, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, contrast, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, saturation, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, sharpness, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, whiteBalance, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, autoExposure, int);
    WRITE_KEY_SUBSECTION(cfg.calib, cfgCameraParametersSection, autoWhiteBalancing, int);

    LOG_INFO << getBlackboardName();
    LOG_INFO << this;

    return true;
}

std::ostream &operator<<(std::ostream &s, const CameraParametersBlackboard *rhs) {
    s << "current camera parameters blackboard content:\n";
    s << "  exposureTop:        " << rhs->exposureTop << "\n";
    s << "  exposureBottom:     " << rhs->exposureBottom << "\n";
    s << "  brightnessTop:      " << rhs->brightnessTop << "\n";
    s << "  brightnessBottom:   " << rhs->brightnessBottom << "\n";
    s << "  gain:               " << rhs->gain << "\n";
    s << "  contrast:           " << rhs->contrast << "\n";
    s << "  saturation:         " << rhs->saturation << "\n";
    s << "  sharpness:          " << rhs->sharpness << "\n";
    s << "  whiteBalance:       " << rhs->whiteBalance << "\n";
    s << "  autoExposure:       " << rhs->autoExposure << "\n";
    s << "  autoWhiteBalancing: " << rhs->autoWhiteBalancing;
    return s;
}
