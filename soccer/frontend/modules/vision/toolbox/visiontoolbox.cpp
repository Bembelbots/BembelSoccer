/**
 * @author Module owner: Bembelbots Frankfurt, andreasf
 *
 * $Id$
 *
 */

#include "visiontoolbox.h"

#include <representations/vision/visiondefinitions.h>
#include <representations/camera/camera.h>

//#include <core/util/platform.h>
//#include <core/util/constants.h>
//#include <shared/common/benchmark/benchmarking.h>
#include <cstdint>
#include <cstring>
#include <vector>

#include <framework/logger/logger.h>
#include <framework/util/assert.h>
#include <framework/util/clock.h>
#include <representations/bembelbots/constants.h>

//only for drawing roi
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../detector/crossing_detector.h"
#include "../detector/ball_detector.h"

using namespace std;

static bool check_vision_result(const CamImage &_img, const VisionResult &vr){
    if(vr.ics_x1 < 0 || size_t(vr.ics_x1) >= _img.width || vr.ics_y1 < 0 || size_t(vr.ics_y1) >= _img.height){
        return false;
    }

    if(vr.ics_x2 < 0 || size_t(vr.ics_x2) >= _img.width || vr.ics_y2 < 0 || size_t(vr.ics_y2) >= _img.height){
        return false;
    }

    return true;
}

static inline void setY(uint8_t *const img, const uint32_t width, const int32_t x,
                 int32_t y, const uint8_t c) {
    img[(x+y*width)<<1]=c;
}

static inline void drawLine(uint8_t *dest, int x1, int y1, int x2, int y2) {

    for (float d=0; d<=1; d+=0.001) {
        int px = x1;
        int py = y1;

        if (x2 && y2) {
            px=(int)round(x1*(1-d)+x2*d);
            py=(int)round(y1*(1-d)+y2*d);
        }

        if (px < 0 || py < 0 || px >= camera::w || py >= camera::h) {
            continue;
        }

        setY(dest, camera::w, px, py, 0);
    }
}

static inline void drawCross(uint8_t *dest, int x, int y, int width, int height) {
    int nx;
    int ny;

    for (int w = -(width / 2); w < (width / 2); ++w) {
        nx = x + w;
        ny = y;

        if (nx < 0|| ny < 0 || nx >= camera::w || ny >= camera::h) {
            continue;
        }

        setY(dest, camera::w, nx, ny, 0);
    }

    for (int h = -(height / 2); h < (height / 2); h += 1) {
        nx = x;
        ny = y + h;

        if (nx < 0|| ny < 0 || nx >= camera::w || ny >= camera::h) {
            continue;
        }

        setY(dest, camera::w, nx, ny, 0);
    }
}


/**
 * constructor.
 */

VisionToolbox::VisionToolbox(const std::string &configPath, int cam) {
    _init(configPath, cam);
}

VisionToolbox::VisionToolbox() {
    _init();
}


VisionToolbox::~VisionToolbox() {
    if (_htwk) {
        // LOG_DEBUG << "Cleanup HTWK vision";
        delete _htwk;
        _htwk = nullptr;
    }

    if (crossingDetector) {
        delete crossingDetector;
        crossingDetector = nullptr;
    }

    if (ballDetector) {
        delete ballDetector;
        ballDetector = nullptr;
    }
}

void VisionToolbox::_init(std::string configPath, int cam) {
	configPath +=  "../nn/";

    // init HTWK Vision

    ballClassifier = configPath + "htwk_vision_ball.net";
    goalClassifier = configPath + "htwk_vision_goal.net";
    robotClassifier = configPath + "htwk_vision_robots.net";

    htwkConfig.objectDetectorPatchSize = 16;
    htwkConfig.objectDetectorNetwork = configPath + "model.prototxt";
    htwkConfig.objectDetectorModel = configPath + "model.caffemodel";
    htwkConfig.isUpperCam = cam == TOP_CAMERA;

    _htwk = new htwk::HTWKVision(camera::w, camera::h, htwkConfig);

    int8_t *lutCb = _htwk->lutCb;
    int8_t *lutCr = _htwk->lutCr;

    // crossing detector 
    crossingDetector = new bbvision::CrossingDetector(camera::w, camera::h, lutCb,
            lutCr);

    // ball detector
    ballDetector = new bbvision::BallDetector2(camera::w, camera::h, _htwk->lutCb,
            _htwk->lutCr, ballClassifier.c_str(), configPath, _htwk->ballFeatureExtractor, htwkConfig);

}

/**
 * processNewImage
 * creates a new binary image and clear it
 * with the dimensions of the given image
 * @param image image to get size from
 */
void VisionToolbox::process(const CamImage &img) {
    _img = img;
    _htwkImg = _img.data;

    jsassert(img.width == camera::w);
    jsassert(img.height == camera::h);

    // set counts to zero.
    // this values are used to start our finding algorithms
    _roi.upperBorder.resize(camera::w + 1);

    jsassert(_htwkImg != nullptr);

    // basic init of htwk vision process
    _htwk->fieldColorDetector->proceed(_htwkImg);
    _htwk->regionClassifier->proceed(_htwkImg, _htwk->fieldColorDetector);

    // 10 iterations improves ball detection. I don't know why yet :/
    //for(size_t i = 0; i < 10; i++){
    _htwk->fieldDetector->proceed(_htwkImg, _htwk->fieldColorDetector,
                                  _htwk->regionClassifier, img.camera == TOP_CAMERA);
    //}

    _htwk->integralImage->proceed(_htwkImg);

}

bool VisionToolbox::isBallFound() const {
    return ballDetector->isBallFound();
}

RoiDef VisionToolbox::getROI() {
    const int *fieldBorder = _htwk->fieldDetector->getConvexFieldBorder();
    // set ROI
    RoiDef roi;
    roi.first.resize(camera::w);
    roi.second.resize(camera::w);
    for (int x=0; x < camera::w; x++) {
        int y=fieldBorder[x];
        if (y < 0 || y >= camera::h) {
            continue;
        }
        roi.first[x] = x;
        roi.second[x] = y;
    }

    return roi;
}

void VisionToolbox::setROI(const CamImage &img, const RoiDef &roi) {
    _roi.startX = 0;
    _roi.endX = img.width + 1;
    _roi.lowerBorder = 0;
    for (size_t i = 0; i < roi.first.size(); ++i) {
        _roi.upperBorder[roi.first[i]] = roi.second[i];
    }
}


void VisionToolbox::drawROI(const CamImage &img) {
    cv::Mat cv_img = img.mat();
    for (int x = _roi.startX; x < _roi.endX; x++) {
        cv::rectangle(cv_img,
                      cv::Point(x - 1, _roi.upperBorder[x] - 1),
                      cv::Point(x + 1, _roi.upperBorder[x] + 1),
                      cv::Scalar(PERFECT_COLOR_ROI), 1, 0);
    }
}

void VisionToolbox::enableExpBallDetection(const bool &enable) {
    _expBallDetection = enable;
}

YuvPixel VisionToolbox::getGreen() {
    auto pixel = _htwk->fieldColorDetector->getColor();
    return {pixel.cy, pixel.cb, pixel.cr};
}

YuvPixel VisionToolbox::getWhite() {
    auto pixel = _htwk->lineDetector->getColor();
    return {pixel.cy, pixel.cb, pixel.cr};
}

void VisionToolbox::drawField(const CamImage &img) {
    //cv::Mat cv_img = img.mat();
    for (size_t x = 0; x < img.width; x += 1) {
        for (size_t y = 0; y < img.height; y += 1) {
            int cy = _htwk->fieldColorDetector->getY(img.data, x, y);
            int cb = _htwk->fieldColorDetector->getCb(img.data, x, y);
            int cr = _htwk->fieldColorDetector->getCr(img.data, x, y);
            if (_htwk->fieldColorDetector->isGreen(cy, cb, cr)) {
                setY(img.data, img.width, x, y, 0); //PERFECT_COLOR_FIELD);
            }
        }
    }
}

std::vector<VisionResult> VisionToolbox::findLines(const bool
        &showScanpoints) {
    std::vector<VisionResult> vrs;

    // get segments on field
    std::vector<htwk::LineSegment *> lineSegments =
        _htwk->regionClassifier->getLineSegments(
            _htwk->fieldDetector->getConvexFieldBorder());

    // do htwk line detection
    _htwk->lineDetector->proceed(_htwkImg,
                                 lineSegments,
                                 _htwk->regionClassifier->lineSpacing);
    
    VisionResult vr;
    vr.type = JSVISION_LINE;
    vr.timestamp = getTimestampMs();

    // convert detected lines to VisionResults
    std::vector<htwk::LineGroup> lines=_htwk->lineDetector->getLineGroups();

    for (auto &it: lines) {

        
        auto &l0 = it.lines[0];
        auto &l1 = it.lines[1];

        //Determine which point on lines[0] is closer to point 1 on lines[1]
        //vector from lines[0] point 1 to lines[1] point 1
        float v1x = (l1.px1 - l0.px1);
        float v1y = (l1.py1 - l0.py1);
        float distance1 = v1x*v1x + v1y*v1y;

        //vector from lines[0] point 2 to lines [1] point 1
        float v2x = (l1.px1 - l0.px2);
        float v2y = (l1.py1 - l0.py2);
        float distance2 = v2x*v2x + v2y*v2y;

        if (distance1 > distance2){
            vr.ics_x1 = (int)((l0.px1 + l1.px2) / 2);
            vr.ics_y1 = (int)((l0.py1 + l1.py2) / 2);
            vr.ics_x2 = (int)((l0.px2 + l1.px1) / 2);
            vr.ics_y2 = (int)((l0.py2 + l1.py1) / 2);

        } 
        else {
            vr.ics_x1 = (int)((l0.px1 + l1.px1) / 2);
            vr.ics_y1 = (int)((l0.py1 + l1.py1) / 2);
            vr.ics_x2 = (int)((l0.px2 + l1.px2) / 2);
            vr.ics_y2 = (int)((l0.py2 + l1.py2) / 2);

        }
    
        _addRcsPositionToVisionResult(vr, true);
        vrs.push_back(vr);
    }
    
    return vrs;
}

std::vector<VisionResult> VisionToolbox::findGoal() {
    std::vector<VisionResult> vrs;

    _htwk->goalDetector->proceed(_htwkImg,
                                 _htwk->fieldDetector->getConvexFieldBorder(),
                                 _htwk->fieldColorDetector->getColor(),
                                 _htwk->lineDetector->getColor());

    std::vector<htwk::GoalPost> posts = _htwk->goalDetector->getGoalPosts();
    if (posts.size() > 1) {
        std::vector<htwk::GoalPost> usePosts;
        if (posts.size() == 2) {
            usePosts = posts;
        } else {
            for (std::vector<htwk::GoalPost>::iterator it = posts.begin();
                    it != posts.end();
                    ++it) {
                if (usePosts.size() < 2) {
                    usePosts.push_back(*it);
                } else {
                    if (it->probability > usePosts[0].probability) {
                        usePosts[0] = *it;
                    } else if (it->probability > usePosts[1].probability) {
                        usePosts[1] = *it;
                    }
                }
            }
        }

        VisionResult pole1;
        pole1.type = JSVISION_GOAL;
        pole1.ics_x1 = usePosts[0].basePoint.x;
        pole1.ics_y1 = usePosts[0].basePoint.y;
        pole1.ics_width = abs(usePosts[0].basePoint.x - usePosts[0].upperPoint.x);
        pole1.ics_height = abs(usePosts[0].basePoint.y - usePosts[0].upperPoint.y);
        pole1.timestamp = getTimestampMs();
        _addRcsPositionToVisionResult(pole1);

        VisionResult pole2;
        pole2.type = JSVISION_GOAL;
        pole2.ics_x1 = usePosts[1].basePoint.x;
        pole2.ics_y1 = usePosts[1].basePoint.y;
        pole2.ics_width = abs(usePosts[1].basePoint.x - usePosts[1].upperPoint.x);
        pole2.ics_height = abs(usePosts[1].basePoint.y - usePosts[1].upperPoint.y);
        pole2.timestamp = getTimestampMs();
        _addRcsPositionToVisionResult(pole2);

        vrs.push_back(pole1);
        vrs.push_back(pole2);
    }

    return vrs;
}

/**
 * findBall
 */
vector<VisionResult> VisionToolbox::findBall(const float &camPitch, const float &camRoll, const int &whiteThreshold) {
    vector<VisionResult> vrs;

    _htwk->hypothesesGenerator->proceed(_htwkImg,
                                        _htwk->fieldDetector->getConvexFieldBorder(),
                                        camPitch,
                                        camRoll,
                                        _htwk->integralImage);
    auto hypotheses = _htwk->hypothesesGenerator->getHypotheses();
    
    if(_img.camera == BOTTOM_CAMERA) { //bottom cam
        ballDetector->proceed(_img, hypotheses, 8);
    } else { //top cam
        ballDetector->proceed(_img, hypotheses, 24);
    }

    // Ball to VisionResult
    if (ballDetector->isBallFound()) {
        auto ball = ballDetector->getBall();
        int bx = ball.x;
        int by = ball.y;
        int r = ball.r;
        jsassert(r>0);
        jsassert(bx>=0);
        jsassert(by>=0);
        VisionResult vr;
        vr.type = JSVISION_BALL;
        vr.ics_x1 = bx;
        vr.ics_y1 = by;
        vr.ics_height = 2*r;
        vr.ics_width = 2*r;
        vr.timestamp = getTimestampMs();
        vr.camera = _img.camera;
        if(check_vision_result(_img, vr)){
            _addRcsPositionToVisionResult(vr);
            vrs.push_back(vr);
        }
    }

    // PenaltyMark to VisionResult
    if(ballDetector->isPenaltyMarkFound()) {
        auto penaltyMark = ballDetector->getPenaltyMark();
        VisionResult vr;
        vr.type = JSVISION_PENALTY;
        vr.ics_x1 = penaltyMark.x;
        vr.ics_y1 = penaltyMark.y;
        vr.ics_height = 10; // arbitrary number for debug display
        vr.ics_width = 10; // arbitrary number for debug display
        vr.timestamp = getTimestampMs();
        if(check_vision_result(_img, vr)){
            _addRcsPositionToVisionResult(vr);
            vrs.push_back(vr);
        }
    }

    // PenaltyMark to VisionResult
    if(ballDetector->isRobotFeetFound()) {
        std::vector<htwk::ObjectHypothesis> robotFeet = ballDetector->getRobotFeet();
        for(htwk::ObjectHypothesis robot : robotFeet){
            VisionResult vr;
            vr.type = JSVISION_ROBOT;
            vr.ics_x1 = robot.x - robot.r;
            vr.ics_y1 = robot.y - robot.r; //we want a bottom line
            vr.ics_height = 2*robot.r; // arbitrary height
            vr.ics_width = 2*robot.r;
            vr.timestamp = getTimestampMs();
            if(check_vision_result(_img, vr)){
                _addRcsPositionToVisionResult(vr);
                vrs.push_back(vr);
            }
        }
    }

    // NNRois to VisionResult
    for(auto nnRoi : ballDetector->getNNRois()){
    	VisionResult vr;
    	vr.type = JSVISION_RECT;
    	vr.ics_x1 = nnRoi.x - nnRoi.r;
    	vr.ics_y1 = nnRoi.y - nnRoi.r;
    	vr.ics_height = 2*nnRoi.r;
    	vr.ics_width = 2*nnRoi.r;
    	vr.timestamp = getTimestampMs();
    	vrs.push_back(vr);	
    }

    return vrs;
}

std::vector<VisionResult> VisionToolbox::findCrossings(
    std::vector<VisionResult>& lineVrs, float center_circle_radius) {

    std::vector<VisionResult> vrs;

    _htwk->ellipseFitter->proceed(
        _htwk->regionClassifier->getLineSegments(
            _htwk->fieldDetector->getConvexFieldBorder()));

    crossingDetector->proceed(
        _img,
        _htwk->lineDetector->getLineGroups(),
        lineVrs,
        _htwk->lineDetector->getColor(),
        _htwk->ellipseFitter,
        center_circle_radius);

    for (auto crossing : crossingDetector->getLCrossings()) {
        VisionResult vr;

        vr.type = JSVISION_LCROSS;
        vr.ics_x1 = crossing.px1;
        vr.ics_y1 = crossing.py1;
        vr.ics_x2 = crossing.px2; // debug
        vr.ics_y2 = crossing.py2; // debug
        vr.extra_float = crossing.orientation;
        vr.timestamp = getTimestampMs();
        if(check_vision_result(_img, vr)){
            _addRcsPositionToVisionResult(vr);
            vrs.push_back(vr);
        }
    }

    for (auto crossing : crossingDetector->getTCrossings()) {
        VisionResult vr;

        vr.type = JSVISION_TCROSS;
        vr.ics_x1 = crossing.px1;
        vr.ics_y1 = crossing.py1;
        vr.ics_x2 = crossing.px2; // debug
        vr.ics_y2 = crossing.py2; // debug
        vr.extra_float = crossing.orientation;
        vr.timestamp = getTimestampMs();
        if(check_vision_result(_img, vr)){
            _addRcsPositionToVisionResult(vr);
            vrs.push_back(vr);
        }
    }

    for (auto crossing : crossingDetector->getCenterFieldCrossings()) {
        VisionResult vr;

        vr.type = JSVISION_XCROSS;
        vr.ics_x1 = crossing.px1;
        vr.ics_y1 = crossing.py1;
        vr.ics_x2 = crossing.px2;
        vr.ics_y2 = crossing.py2;
        vr.timestamp = getTimestampMs();
        vr.rcs_confidence = 0.9f;

        if (!crossing.has_rcs){
            if(check_vision_result(_img, vr)){
                _addRcsPositionToVisionResult(vr, true);
                vr.rcs_x1 = crossing.rcs_x1;
                vr.rcs_y1 = crossing.rcs_y1;
                vr.rcs_alpha = crossing.rcs_alpha;
                vr.rcs_distance = crossing.rcs_distance;
                vrs.push_back(vr);
            }
        } else {
            vr.rcs_x1 = crossing.rcs_x1;
            vr.rcs_y1 = crossing.rcs_y1;
            vr.rcs_alpha = crossing.rcs_alpha;
            vr.rcs_distance = crossing.rcs_distance;
            vrs.push_back(vr);
        }
    }

    auto centerCirclePoint = crossingDetector->getCenterCirclePoint();
    if(centerCirclePoint.found){
        VisionResult vr;

        vr.type = JSVISION_CIRCLE;
        vr.ics_x1 = centerCirclePoint.px1;
        vr.ics_y1 = centerCirclePoint.py1;
        vr.timestamp = getTimestampMs();
        vr.rcs_confidence = 1.0f;

        if(!centerCirclePoint.has_rcs){
            if(check_vision_result(_img, vr)){
                _addRcsPositionToVisionResult(vr, false);
                vrs.push_back(vr);
            }
        }        
    }

    return vrs;
}

std::vector<VisionResult> VisionToolbox::findRobots() {
    std::vector<VisionResult> vrs;

    //disabled for now
    return vrs;

    /*_htwk->ellipseFitter->proceed(
            _htwk->regionClassifier->getLineSegments(_htwk->fieldDetector->getConvexFieldBorder()),
            _htwk->lineDetector->lineEdges);

    _htwk->resultRobotClassifier.clear();

    _htwk->robotAreaDetector->proceed(_htwkImg,
            _htwk->regionClassifier->getScanVertical(),
            _htwk->fieldDetector->getConvexFieldBorder(),
            _htwk->fieldColorDetector,
            _htwk->ballDetector->getBall(),
            _htwk->goalDetector->getGoalPosts());

    int rectCounter=0;

    //for(const Rect &rect : *robotAreaDetector->getRobotAreas()) {
    for (std::vector<htwk::RobotRect>::iterator it = _htwk->robotAreaDetector->getRobotAreas()->begin();
            it != _htwk->robotAreaDetector->getRobotAreas()->end();
            ++it) {
        htwk::RobotRect &rect = *it;
        rectCounter++;

        htwk::RobotClassifierResult res;
        _htwk->robotClassifier->proceed(_htwkImg, rect, res);

        if(res.detectionProbability > 0.125f){
            _htwk->resultRobotClassifier.push_back(res);
            VisionResult vr;
            vr.ics_y1 = rect.yBottom;
            vr.ics_width = rect.xRight - rect.xLeft;
            vr.ics_x1 = rect.xLeft+(vr.ics_width/2);
            vr.ics_height = rect.yBottom - rect.yTop;
            vr.timestamp = getTimestampMs();
            vr.type = JSVISION_ROBOT;
            vrs.push_back(vr);
        }
    }

    return vrs; */
}

void VisionToolbox::_addRcsPositionToVisionResult(VisionResult &vr,
        bool bothPositions) {
    Coord rcs = _img.getRcsPosition(vr.ics_x1, vr.ics_y1);

    vr.rcs_x1 = rcs.x;
    vr.rcs_y1 = rcs.y;
    vr.rcs_distance = rcs.dist();
    vr.rcs_alpha = rcs.angle().rad();

    if (bothPositions == false) {
        return;
    }

    rcs = _img.getRcsPosition(vr.ics_x2, vr.ics_y2);

    vr.rcs_x2 = rcs.x;
    vr.rcs_y2 = rcs.y;
}

YuvImage VisionToolbox::drawVisionResults(std::vector<VisionResult> &vrs) {
    int size = sizeof(uint8_t) * camera::w * camera::h * 2;
    //uint8_t *dest = (uint8_t*) malloc(size);
    uint8_t *dest = nullptr;
    int r = posix_memalign(reinterpret_cast<void **>(&dest), 8, sizeof(uint8_t)*size);
    jsassert(r == 0);
    memcpy(dest, _htwkImg, size);

    for (size_t i = 0; i < vrs.size(); ++i) {
        VisionResult vr = vrs[i];

        bool hasWidthOrHeight = vr.ics_width || vr.ics_height;

        // It is a simple line

        if (vr.ics_x2 && vr.ics_y2 && !hasWidthOrHeight) {
            drawLine(dest, vr.ics_x1, vr.ics_y1, vr.ics_x2, vr.ics_y2);
            continue;
        }

        // It is a single point

        if (!hasWidthOrHeight) {
            drawCross(dest, vr.ics_x1, vr.ics_y1, 20, 20);
            continue;
        }

        // It is a block

        if (vr.ics_x2 && vr.ics_y2) {

            int half_height = vr.ics_height / 2;
            int half_width = vr.ics_width / 2;

            int x1 = vr.ics_x1 - half_width;
            int y1 = vr.ics_y1 - half_height;

            int x2 = vr.ics_x2 - half_width;
            int y2 = vr.ics_y2 - half_height;

            int x3 = vr.ics_x1 - half_width;
            int y3 = vr.ics_y1 + half_height;

            int x4 = vr.ics_x2 - half_width;
            int y4 = vr.ics_y2 + half_height;

            int x5 = vr.ics_x1 + half_width;
            int y5 = vr.ics_y1 - half_height;

            int x6 = vr.ics_x2 + half_width;
            int y6 = vr.ics_y2 - half_height;

            int x7 = vr.ics_x1 + half_width;
            int y7 = vr.ics_y1 + half_height;

            int x8 = vr.ics_x2 + half_width;
            int y8 = vr.ics_y2 + half_height;

            //draw first rectangle
            drawLine(dest, x1, y1, x2, y2);
            drawLine(dest, x3, y3, x4, y4);

            drawLine(dest, x1, y1, x3, y3);
            drawLine(dest, x2, y2, x4, y4);

            //draw second rectangle

            drawLine(dest, x5, y5, x6, y6);
            drawLine(dest, x7, y7, x8, y8);

            drawLine(dest, x5, y5, x7, y7);
            drawLine(dest, x6, y6, x8, y8);

            //connect the rectangles

            drawLine(dest, x1, y1, x5, y5);
            drawLine(dest, x3, y3, x7, y7);

            drawLine(dest, x2, y2, x6, y6);
            drawLine(dest, x4, y4, x8, y8);

            continue;
        }

        // It is also a line

        if (!(vr.ics_width && vr.ics_height)) {
            drawLine(dest, vr.ics_x1, vr.ics_y1, vr.ics_x2, vr.ics_y2);
            continue;
        }

        // It is a circle

        drawCross(dest, vr.ics_x1, vr.ics_y1, vr.ics_width, vr.ics_height);
    }

    return YuvImage(dest, camera::w, camera::h);
}
