#pragma once

//#define CPU_ONLY
#ifndef USE_OPENCV
#define USE_OPENCV
#endif

#include <caffe/caffe.hpp>
#ifdef USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif  // USE_OPENCV
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>
#include <glob.h>
#include <sys/stat.h>
#include <ctime>

class CaffeClassifier {
public:
    CaffeClassifier(const std::string& model_file,
               const std::string& trained_file, const std::string& preprocess_file);

    std::vector<float> Classify(const cv::Mat &img, int N = 5);

    static std::vector<int> Argmax(const std::vector<float>& v, int N);

    void setMean(float mean);
    void setStddev(float stddev);

    cv::Size input_geometry_;

    std::shared_ptr<caffe::Net<float>> net_;
    void WrapInputLayer(std::vector<cv::Mat>* input_channels);

    void Preprocess(const cv::Mat &img,
                    std::vector<cv::Mat> *input_channels);
private:
    //cv::Size input_geometry_;
    int num_channels_;
    float mean;

    float stddev;

    std::vector<float> Predict(const cv::Mat& img);

    static bool PairCompare(const std::pair<float, int>& lhs, const std::pair<float, int>& rhs);

    void LoadPreprocessParamters(const std::string& preprocess_file);

};
