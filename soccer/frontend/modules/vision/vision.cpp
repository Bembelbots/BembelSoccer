#include "vision.h"
#include <framework/logger/logger.h>
#include <representations/playingfield/playingfield.h>
#include "toolbox/visiontoolbox.h"
#include "toolbox/colorclasses.h"
#include <future>

static void visionResultsAppend(VisionResultVec &results, VisionResultVec &other) {
    results.insert(results.end(), other.begin(), other.end());
}


void Vision::connect(rt::Linker &link) {
    link.name = "Vision";
    link(settings);
    link(image_provider);
    link(processed);
    link(playingfield);
    link(vision_results);
    link(nao);
}

void Vision::setup() {
    top_toolbox = std::make_shared<VisionToolbox>(settings->configPath, TOP_CAMERA);
    bottom_toolbox = std::make_shared<VisionToolbox>(settings->configPath, BOTTOM_CAMERA);
    center_circle_radius = playingfield->_circle.wcs_radius;
}

CamImage Vision::getImage(int cam) {
    CamImage img = image_provider->getImage(cam);
    img.lock(ImgLock::VISION);
    return img;
}

void Vision::process() {
    auto lock = board.scopedLock();
    std::future<CamImage> topimg, botimg;
    topimg = std::async(&Vision::getImage, this, TOP_CAMERA);
    botimg = std::async(&Vision::getImage, this, BOTTOM_CAMERA);
    
    std::future<DetectResult> top_detect, bottom_detect;
    top_detect = std::async(&Vision::processTopCam, this, topimg.get());
    bottom_detect = std::async(&Vision::processBottomCam, this, botimg.get());

    auto [top_ball, top_results] = top_detect.get(); 
    auto [bottom_ball, bottom_results] = bottom_detect.get();
    
    // verify camera of results
    for(auto &result : top_results) {
        jsassert(result.camera == TOP_CAMERA);
    }

    // fix bottom camera results
    for(auto &result : bottom_results) {
        //jsassert(result.camera == BOTTOM_CAMERA);
        result.camera = BOTTOM_CAMERA;
    }

    //LOG_DEBUG_IF(top_ball || bottom_ball) << "Ball Found!";
    if(top_ball && bottom_ball) {
        for(auto it = top_results.begin(); it != top_results.end();){
            if ((*it).type == JSVISION_BALL) {
                top_results.erase(it);
            } else{
                it++;
            }
        }
    }

    vision_results->clear();
    visionResultsAppend(*vision_results, bottom_results);
    visionResultsAppend(*vision_results, top_results);

    board._visionResults = *vision_results;
}

Vision::DetectResult Vision::processTopCam(CamImage img){
    auto &toolbox = *top_toolbox;
    jsassert(img.camera == TOP_CAMERA);
    toolbox.process(img);
    
    // calucalte ROI
    RoiDef roi = toolbox.getROI();

    // set it and draw it on request
    toolbox.setROI(img, roi);
    if (board.showROI == 1) {
        toolbox.drawROI(img);
    }

    // for debugging, draw field on the image
    if (board.showField) {
        toolbox.drawField(img);
    }

    ColorClass::green = toolbox.getGreen();
    ColorClass::white = toolbox.getWhite();
    
    auto res = detect(img, toolbox);

    if (board.ballDistanceMeasuredTop > 0) {
        autoCalibratePitchWithBall(board.ballDistanceMeasuredTop, img, res.second);
        board.ballDistanceMeasuredTop = 0;
    }
 
    return res;
}

Vision::DetectResult Vision::processBottomCam(CamImage img){
    auto &toolbox = *bottom_toolbox;
    jsassert(img.camera == BOTTOM_CAMERA);
    toolbox.process(img);

    auto res = detect(img, toolbox);

    if (board.ballDistanceMeasuredBottom > 0) {
        autoCalibratePitchWithBall(board.ballDistanceMeasuredBottom, img, res.second);
        board.ballDistanceMeasuredBottom = 0;
    }

    return res;
}

Vision::DetectResult Vision::detect(CamImage &img, VisionToolbox &toolbox) {
    VisionResultVec results;

    const camPose &eulers = img._eulers;
    auto ball = toolbox.findBall(eulers.r[1], eulers.r[2]);
    visionResultsAppend(results, ball);
    auto lines = toolbox.findLines(board.showScanpoints);
    visionResultsAppend(results, lines);
    auto crossings = toolbox.findCrossings(lines, center_circle_radius);
    visionResultsAppend(results, crossings);


    auto processed_img = VisionImageProcessed(img);
    processed_img.results = results;
    processed.emit(processed_img);

    img.unlock(ImgLock::VISION);
    return std::make_pair(toolbox.isBallFound(), results);
}

void Vision::autoCalibratePitchWithBall(float real_distance, CamImage &image, VisionResultVec &vrs) {
    size_t ball = 0;
    
    bool ballFound = false;
    for (size_t i = 0; i < vrs.size(); ++i) {
        if (vrs[i].type == JSVISION_BALL) {
            ball = i;
            ballFound = true;
        }
    }

    // rcs = robot coordinate system
    if (ballFound) {
        float rcs_distance = vrs[ball].rcs_x1;
        float height = image._eulers.v(2);
        
        float current_angle = atan2(rcs_distance, height);
        float real_angle = atan2(real_distance, height);
        float pitch_offset = current_angle - real_angle;

        nao.enqueue<SetPitchOffset>(image.camera, pitch_offset);
        
        // LOG_DEBUG <<"Current angle: " << current_angle;
        // LOG_DEBUG <<"Real angle: " << real_angle;
        // LOG_DEBUG <<"Pitch offset: " << pitch_offset;
        // LOG_DEBUG <<"vision board after: " << board.bottomCameraCalibration->pitchCorrection;
    }
}

void Vision::stop() {
    LOG_DEBUG << "Vision Stop";
}

