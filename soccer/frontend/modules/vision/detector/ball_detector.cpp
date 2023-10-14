//
// Created by kito on 5/3/18.
//

#include "ball_detector.h"

#include <framework/logger/logger.h>
#include <framework/util/assert.h>
#include <representations/bembelbots/constants.h>
#include <HTWKVision/ball_feature_extractor.h>
#include <caffe/caffe.hpp>
#include "caffeclassifier.h"

/*
#if BB_VISION_PATCHES
#include <core/util/tafel.h>
#include <core/settingsblackboard.h>
#endif
*/

namespace bbvision {
    BallDetector2::BallDetector2(const int _width, const int _height, const int8_t *_lutCb, const int8_t *_lutCr,
                                 const char *classifierName, const std::string &configPath,
                                 htwk::BallFeatureExtractor *_featureExtractor, const htwk::HtwkVisionConfig &config)
            : BaseDetector(_width, _height, _lutCb, _lutCr),
              configPath(configPath),
              MIN_BALL_PROB(config.ballProbabilityThreshold),
              FEATURE_SIZE(config.objectDetectorPatchSize),
              foundBall(false), featureExtractor(_featureExtractor){

        caffeClassifier = newCaffeClassifier("BallNet_Sydney");
        penaltyClassifier = newCaffeClassifier("RoboFeetNet_1");

        LOG_DEBUG << "INIT BALLDETECTOR 2";
        LOG_DEBUG << caffeClassifier->input_geometry_;

        LOG_DEBUG << "INIT PENALTY MODEL";
        LOG_DEBUG << penaltyClassifier->input_geometry_;
    }


    CaffeClassifier *BallDetector2::newCaffeClassifier(const std::string &basename) {
		jsassert(!basename.empty());
		std::string fname(configPath + basename);
        return new CaffeClassifier(fname + ".prototxt", fname + ".caffemodel", fname + ".csv");
		
	}


/*
 * detects the ball (if visible) and outputs its position
 */
    // TODO: propagate that CamImage is not const anymore
    void BallDetector2::proceed(CamImage& img, std::vector<htwk::ObjectHypothesis> &hypoList, int max_trys)
    {
        nnRois.clear();

        ratedBallHypotheses.clear();
        robotFeetHypothesis.clear();

        foundBall = false;
        penaltyMarkFound = false;

        int try_count = 0;
        for(htwk::ObjectHypothesis& hyp : hypoList){
            if(try_count >= max_trys){
                break;
            }
            try_count += 1;
                        
            if(img.camera == TOP_CAMERA){ // top camera
                hyp.r = estimatedBallRadius(hyp.x, hyp.y, img);
            } else {
                hyp.r += 15;
            }

            nnRois.push_back(hyp);

            int topLeftX = std::max<int>(0, hyp.x - hyp.r);
            int topLeftY = std::max<int>(0, hyp.y - hyp.r);
            int bottomRightX = std::min<int>(width,  hyp.x + hyp.r);
            int bottomRightY = std::min<int>(height,  hyp.y + hyp.r);

            auto patch = cv::Rect(topLeftX, topLeftY, bottomRightX - topLeftX, bottomRightY - topLeftY);

            // Prepare image patch for network pass
            cv::Mat currImage = cv::Mat(patch.height, patch.width, CV_8UC1, 1);
            cutCyFromImage(img, currImage, patch);
            
            // Pass through BallNet
            float ballProb = 0.f;
            if(!foundBall){
            	std::vector<float> result = caffeClassifier->Classify(currImage, 1);
            	ballProb = result[1];

                // Convert x,y to RCS position
                //Coord ballRcs = img.getRcsPosition(hyp.x, hyp.y); // RCS positon of the ball
                
                // Get distance to rcs position
                //float dist = ballRcs.dist();
                //LOG_DEBUG_EVERY_N(100) << "dist: " << dist;
            }
            
            // Pass through PenaltyRobotNet
            float penaltyProb = 0.f;
            float robotProb = 0.f;
            if(hyp.y >= 0.3*img.height){//hyp.y >= 0.3*img.height){ // Check if roi is below theshold
                std::vector<float> resultPenalty = penaltyClassifier->Classify(currImage, 1);
                //penaltyProb = resultPenalty[1];
                robotProb = resultPenalty[1];
            }

            ratedBallHypotheses.push_back(hyp);

/*
//WRITE PATCHES:
#if BB_VISION_PATCHES
            int startx = topLeftX;
            int starty = topLeftY;
            int w = bottomRightX - topLeftX;
            int h = bottomRightY - topLeftY;

            IplImage *snap = cvCreateImage(cvSize(w, h), 8, 3);
            cvZero(snap);
            for (int y=0; y<h; ++y) {
                for (int x=0; x<w; ++x) {
                    int ox = startx + x;
                    int oy = starty + y;
                    uint8_t cy = getY(img.data, ox, oy);
                    uint8_t cu = getCb(img.data, ox, oy);
                    uint8_t cv = getCr(img.data, ox, oy);

                    ((uchar *)(snap->imageData + snap->widthStep * y))[x*3 + 0] = cy;
                    ((uchar *)(snap->imageData + snap->widthStep * y))[x*3 + 1] = cu;
                    ((uchar *)(snap->imageData + snap->widthStep * y))[x*3 + 2] = cv;
                }
            }

            std::stringstream imageFile;

            imageFile << settings()->imagePath << "/patches/img" << std::setw(8) << std::setfill('0') <<
                      getTimestampMs();

            // write patch
            if(ballProb > MIN_BALL_PROB){
                imageFile << "-ball.png";
                //LOG_DEBUG << "write ball patch" << w << "," << h << ") to " << imageFile.str() << " saved";
                //cvSaveImage(imageFile.str().c_str(), snap);
            }
            else if(penaltyProb > 0.2f){

                imageFile << "-pMark.png";
                //LOG_DEBUG << "writing penalty mark" << w << "," << h << ") to " << imageFile.str() << " saved";
                cvSaveImage(imageFile.str().c_str(), snap);
            }

            else if(robotProb > 0.3f){
                imageFile << "-robot.png";
                //LOG_DEBUG << "writing penalty mark" << w << "," << h << ") to " << imageFile.str() << " saved";
                cvSaveImage(imageFile.str().c_str(), snap);
            }
            else{
                imageFile << "-bg.png";
                //cvSaveImage(imageFile.str().c_str(), snap);
            }
            //if(img.camera == 1){
            //    LOG_DEBUG << "patch with size (" << w << "," << h << ") to " << imageFile.str() << " saved";
            //    cvSaveImage(imageFile.str().c_str(), snap);    
            //    imgCounter = 0;
            //}
            imgCounter++;
            cvReleaseImage(&snap);
#endif
*/

            // save Ball
            if (ballProb > 0.99f) {
                hyp.prob = ballProb;
                bestBallHypothesis=hyp;
                foundBall = true;
                //break;
            } 
            else if(penaltyProb > MIN_BALL_PROB) { 
                hyp.prob = penaltyProb;
                penaltyMarkHypothesis = hyp;
                penaltyMarkFound = true;
            } 
            else if(robotProb > 0.7f){
                hyp.prob = robotProb;
                robotFeetHypothesis.push_back(hyp);
                robotFeetFound = true;
            }
            
        }

    }

    void BallDetector2::cutCyFromImage(const CamImage &img, cv::Mat &currImage,
                   cv::Rect &boundingBox) {
        assert(boundingBox.x >= 0);
        assert(boundingBox.y >= 0);
        assert(boundingBox.height > 0);
        assert(boundingBox.width > 0);
        assert((size_t)boundingBox.height <= img.height);
        assert((size_t)boundingBox.width <= img.width);
        for (int y=0; y<boundingBox.height; y++) {
            for (int x=0; x<boundingBox.width; x++) {
                currImage.at<uchar>(y, x) = static_cast<uchar>(
                        getY(img.data, (x+boundingBox.x), (y+boundingBox.y)));
            }
        }
    }

    /*
    * Re-Calculate radius of hypothesis.
    * Top Camera only.
    */
    int BallDetector2::estimatedBallRadius(int x, int y, CamImage& img) {
        int radius = 0;
        #if V6
        if(y > 0.1*height){ // fist linear component 
            radius = (int) ((y / (float)height) * 50 + 10); 
        } else{ // second linar component
            radius = (int) ((y / (float)height) * 60 + 12);
        }
        #else
        if(y > 0.1*height){ // fist linear component 
            radius = (int) ((y / (float)height) * 50 + 8); 
        } else{ // second linar component
            radius = (int) ((y / (float)height) * 60 + 8);
        }    
        #endif
        

        return radius;
    }

}
