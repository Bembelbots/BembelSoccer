#pragma once
#include <framework/blackboard/blackboard.h>

class CameraParametersBlackboard : public Blackboard {
public:
    explicit CameraParametersBlackboard();
    ~CameraParametersBlackboard();

    MAKE_VAR(int, exposureTop);
    MAKE_VAR(int, exposureBottom);
    MAKE_VAR(int, brightnessTop);
    MAKE_VAR(int, brightnessBottom);
    MAKE_VAR(int, gain);
    MAKE_VAR(int, contrast);
    MAKE_VAR(int, saturation);
    MAKE_VAR(int, sharpness);
    MAKE_VAR(int, whiteBalance);
    MAKE_VAR(int, autoExposure);
    MAKE_VAR(int, autoWhiteBalancing);

    bool loadConfig(ConfigFiles &) override;
    bool writeConfig(ConfigFiles &) override;

};
