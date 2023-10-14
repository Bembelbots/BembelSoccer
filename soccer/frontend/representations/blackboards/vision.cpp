#include "vision.h"

#include <future>
#include <vector>
#include <algorithm>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <framework/logger/logger.h>
#include <framework/image/rgb.h>
#include <framework/util/configparser.h>

//#include <vision/toolbox/colorclasses.h>

using namespace std;

VisionBlackboard::VisionBlackboard()
  : Blackboard("VisionData") {

    INIT_VAR(_imageTimestamp, 0, "Timestamp when images were last updated");
    INIT_VAR_RW(_frameSkip, 0, "reduce framerate sent to bembelDbug by skipping this many images for every sent image");

    INIT_VAR_RW(highQuality, 0, "do not resize images while debugging");
    INIT_VAR_RW(_losslessImage, 0, "send images uncompressed");
    INIT_SWITCH(_topRawImage, CamImage(), "Top Image (Raw Data)");
    INIT_SWITCH(_bottomRawImage, CamImage(), "bottom Image (Raw Data)");
    INIT_SWITCH(_topClassifiedImage, YuvImage(), "Top Image (Classified Data)");
    INIT_SWITCH(_bottomClassifiedImage, YuvImage(), "Bottom Image (Classified Data)");
    INIT_SWITCH(_topHistogram, YuvImage(), "Top Image Histogram");
    INIT_SWITCH(_bottomHistogram, YuvImage(), "Bottom Image Histogram");

    // set active camera to a false value, so that it will fail,
    // if we call something without init camera!
    INIT_VAR(_activeCamera, -1, "currently active camera");
    INIT_VAR(_visionResults, vector<VisionResult>(), "vision results of last run");
    INIT_VAR_RW(showPitchCorrectionTop, 0, "draw line points into classified image (top)");
    INIT_VAR_RW(showPitchCorrectionBottom, 0, "draw line points into classified image (bottom)");
    INIT_VAR_RW(showScanpoints, 0, "show scanpoints in image");
    INIT_VAR_RW(showROI, 0, "draw roi to image");
    INIT_VAR_RW(showField, 0, "classify field color");
    INIT_VAR_RW(saveImages, false, "save images with foot bumper press");
    INIT_VAR_RW(autoCalibratePitch, 0, "automatically try to set pitch");
    INIT_VAR_RW(expBallDetection, false, "enable the experimental Ball Detection");

    INIT_SWITCH(saveNextTopImage, 0, "request to save next top image");
    INIT_SWITCH(saveNextBottomImage, 0, "request to save next bottom image");
    INIT_VAR(_percentFieldRemainingBottom, 100, "the amount of field remaining in bottom cam");
    INIT_VAR_RW(ball_whiteTreshold, 120, "threshold for white color in ball detection");

    INIT_VAR_RW(ballDistanceMeasuredTop, 0, "measure the ball distance in top cam");
    INIT_VAR_RW(ballDistanceMeasuredBottom, 0, "measure the ball distance in bottom cam");
}

bool VisionBlackboard::loadConfig(ConfigFiles &fcfg) {
    return true;
}

bool VisionBlackboard::writeConfig(ConfigFiles &fcfg) {
    return true;
}

VisionBlackboard::~VisionBlackboard() {
    LOG_INFO << "Cleaning up vision data ...";
}

/*
void VisionBlackboard::applyBackproject(CamImage image) {
    vector<pair<int, int>> linePoints;

    auto euler = image._eulers;

    // get GT position
    //Robot gtPos(modules().get<JsMHKF>()->getRobotPoseWcs());
    Robot mybot;

    // place in the middle of the penalty box!
    mybot.pos = {-(playingfield->_lengthInsideBounds - playingfield->_penaltyLength) / 2.0f, 0.0f, 0_deg};
    linePoints = backprojection->getCurrentOptimalLinePoints(camPoseToArray(euler), 2, mybot);

    for (const auto &linePoint : linePoints) {

        size_t x = linePoint.first;
        size_t y = linePoint.second;

        size_t startx = x - 2;
        size_t endx = min(x + 2, image.width);

        size_t starty = y - 2;
        size_t endy = min(y + 2, image.height);

        for (size_t nx = startx; nx < endx; ++nx) {
            for (size_t ny = starty; ny < endy; ++ny) {
                image.setY(nx, ny, PERFECT_COLOR_PITCH_CORRECTION);
            }
        }
    }
}
*/

ostream &operator<<(ostream &s, VisionBlackboard &rhs) {
    /*
    camPose et = rhs.getTopCamTransform();
    camPose eb = rhs.getBottomCamTransform();

    s << "Top Image:";
    s << "\n\ttimestamp:\t" << rhs.topRawImage().timestamp;
    s << "\nBottom Image:";
    s << "\n\ttimestamp:\t" << rhs.bottomRawImage().timestamp;
    s << "\nSensor Data:";
    s << "\n\ttimestamp:\t" << rhs.getSensorDataTimestamp();
    s << "\n\ttick:\t\t" << rhs.getSensorDataTick();
    s << "\n\tEulers (Top):\t" << et.v[0] << ", " << et.v[1] << ", " << et.v[2];
    s << ", " << et.r[0] << ", " << et.r[1] << ", " << et.r[2];
    s << "\n\tEulers (Bottom):" << eb.v[0] << ", " << eb.v[1] << ", " << eb.v[2];
    s << ", " << eb.r[0] << ", " << eb.r[1] << ", " << eb.r[2];
    */

    return s;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
