#pragma once

#include <cstdint>

#include "base_detector.h"
#include <HTWKVision/neuralnet/classifier.h>
#include <HTWKVision/color.h>
#include <HTWKVision/point_2d.h>
//#include <HTWKVision/region_classifier.h>
#include <HTWKVision/object_hypothesis.h>
//#include <HTWKVision/ball_feature_extractor.h>
#include <HTWKVision/htwk_vision_config.h>
#include <linesegment.h>


//#include <range_check.h>

class FindBallTesting;
class CamImage;
class CaffeClassifier;

namespace htwk {
    class BallFeatureExtractor;
}

namespace bbvision {

    static constexpr int MAX_TRIES_DEFAULT = 10;

    class BallDetector2 : protected bbvision::BaseDetector {

    public:

        BallDetector2(const int _width, const int _height, const int8_t *_lutCb, const int8_t *_lutCr,
                      const char *classifierName, const std::string &configPath,
                htwk::BallFeatureExtractor *_featureExtractor, const htwk::HtwkVisionConfig &config);

        ~BallDetector2() = default;

        void proceed(CamImage & img, std::vector<htwk::ObjectHypothesis> &hypoList, int max_trys = MAX_TRIES_DEFAULT);

        void cutCyFromImage(const CamImage &img, cv::Mat &currImage,
                            cv::Rect &boundingBox);

        int estimatedBallRadius(int x, int y, CamImage& img);
        int estimatedBallRadiusNew(int x, int y, CamImage img);

        float candidateWhiteBlackTendency(htwk::ObjectHypothesis& hyp, int radiusEstimate, const CamImage &image);

        bool isBallFound() const { return foundBall; }
        bool isPenaltyMarkFound() const { return penaltyMarkFound; }
        bool isRobotFeetFound() const { return robotFeetFound; }

        const htwk::ObjectHypothesis& getBall() const { return bestBallHypothesis; }
        const htwk::ObjectHypothesis& getPenaltyMark() const { return penaltyMarkHypothesis; }
        const std::vector<htwk::ObjectHypothesis>& getRobotFeet() const { return robotFeetHypothesis; }
        const std::vector<htwk::ObjectHypothesis>& getNNRois() const { return nnRois; }
        // This is used by the machine learning tools. This must not be used in the firmware!
        const std::vector<htwk::ObjectHypothesis>& getRatedBallHypotheses() const { return ratedBallHypotheses; }

    private:
        std::string configPath;

        CaffeClassifier *newCaffeClassifier(const std::string &basename);

        const float& MIN_BALL_PROB;

        const int FEATURE_SIZE;

        bool foundBall{false};
        bool penaltyMarkFound{false};
        bool robotFeetFound{false};

        std::vector<htwk::ObjectHypothesis> nnRois;

        htwk::ObjectHypothesis bestBallHypothesis;
        htwk::ObjectHypothesis penaltyMarkHypothesis;
        std::vector<htwk::ObjectHypothesis> robotFeetHypothesis;
        std::vector<htwk::ObjectHypothesis> ratedBallHypotheses;

        htwk::BallFeatureExtractor* featureExtractor;

        CaffeClassifier *caffeClassifier;
        CaffeClassifier *penaltyClassifier;

        const int imageSaveModulo = 80;
        int imgCounter = 0;

    };

}  // namespace bbvision

// vim: set ts=4 sw=4 sts=4 expandtab:
