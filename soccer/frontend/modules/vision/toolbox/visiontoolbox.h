#pragma once
#include <HTWKVision/htwk_vision.h>
#include <framework/image/camimage.h>

class VisionResult;

namespace bbvision {
    class CrossingDetector;
    class BallDetector2;
}

// copied from visionvars.h
typedef std::pair< std::vector<int>, std::vector<int> > RoiDef;

class VisionToolbox {
    friend class FindBallTesting;
public:
    std::string ballClassifier;
    std::string goalClassifier;
    std::string robotClassifier;

    VisionToolbox(const std::string &configPath, int cam);
    VisionToolbox();
    ~VisionToolbox();

    /// create new binary image
    void process(const CamImage &img);

    bool isBallFound() const;

    // enable the new Ball Detection
    void enableExpBallDetection(const bool &enable);

    // calculate a ROI in the used image
    RoiDef getROI();

    /// set roi by vectors
    void setROI(const CamImage &img, const RoiDef &roi);

    /// draw Roi into image.
    void drawROI(const CamImage &img);

    /// draw field into image.
    void drawField(const CamImage &img);

    YuvPixel getGreen();
    YuvPixel getWhite();

    // FIND LINES TOPCAM (HTWK)
    std::vector<VisionResult> findLines(const bool &showScanpoints = false);

    // FIND GOAL
    std::vector<VisionResult> findGoal();

    // FIND BALL
    std::vector<VisionResult> findBall(const float &camPitch, const float &camRoll, const int &whiteThreshold = 120);

    // FIND CROSSINGS
    std::vector<VisionResult> findCrossings(std::vector<VisionResult>& lineVrs,
        float center_circle_radius);

    // FIND ROBOTS
    std::vector<VisionResult> findRobots(); // cppcheck-suppress functionStatic

    YuvImage drawVisionResults(std::vector<VisionResult> &vrs);

	/// returns fname prefixed with base path
	std::string getNetPath(std::string fname="");

private:
    bbvision::CrossingDetector *crossingDetector;
    bbvision::BallDetector2 *ballDetector;
    htwk::HtwkVisionConfig htwkConfig;

    bool _expBallDetection = false;

    /// roi-variables
    struct Roi {
        int startX, endX;
        int lowerBorder;
        std::vector<int> upperBorder;
    };

    Roi _roi;

    // htwk vision toolbox
    htwk::HTWKVision *_htwk;

    CamImage _img;

    uint8_t *_htwkImg;

    void _init(std::string configPath="./data/", int cam = 0);


    void _addRcsPositionToVisionResult(VisionResult &vr, bool bothPositions=false);
};
