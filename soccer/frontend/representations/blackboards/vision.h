#pragma once

//#include <core/util/platform.h>

#include <framework/blackboard/blackboard.h>
#include "../vision/visiondefinitions.h"
#include <framework/image/camimage.h>

class VisionBlackboard : public Blackboard {
public:
    explicit VisionBlackboard();
    ~VisionBlackboard();

    /**
     * load calibration from config file
     */
    bool loadConfig(ConfigFiles &) override;

    /**
     * write calibration to config file
     */
    bool writeConfig(ConfigFiles &) override;

    MAKE_VAR(int, showPitchCorrectionTop);
    MAKE_VAR(int, showPitchCorrectionBottom);
    MAKE_VAR(int, showROI);
    MAKE_VAR(bool, showField);
    MAKE_VAR(bool, showScanpoints);
    MAKE_VAR(bool, saveImages);
    MAKE_VAR(bool, saveNextTopImage);
    MAKE_VAR(bool, saveNextBottomImage);
    MAKE_VAR(bool, autoCalibratePitch);
    MAKE_VAR(bool, expBallDetection);
    MAKE_VAR(int, ball_whiteTreshold);
    MAKE_VAR(float, ballDistanceMeasuredTop);
    MAKE_VAR(float, ballDistanceMeasuredBottom);

    MAKE_VAR(std::vector<VisionResult>, _visionResults);

private:

    MAKE_VAR(int, highQuality);
    MAKE_VAR(int, _losslessImage);
    MAKE_VAR(CamImage, _topRawImage);
    MAKE_VAR(CamImage, _bottomRawImage);
    MAKE_VAR(YuvImage, _topClassifiedImage);
    MAKE_VAR(YuvImage, _bottomClassifiedImage);
    MAKE_VAR(YuvImage, _topHistogram);
    MAKE_VAR(YuvImage, _bottomHistogram);

    MAKE_VAR(int, _imageTimestamp);
    MAKE_VAR(int, _frameSkip);

    MAKE_VAR(int, _activeCamera);
    MAKE_VAR(int, _percentFieldRemainingBottom);
};

// vim: set ts=4 sw=4 sts=4 expandtab:
