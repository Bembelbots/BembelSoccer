#include "caffeclassifier.h"

#include <caffe/caffe.hpp>


#include <iosfwd>
#include <fstream>

#include <boost/filesystem.hpp>
#include <glob.h>
#include <sys/stat.h>
#include <ctime>

#ifdef USE_OPENCV
using namespace caffe;  // NOLINT(build/namespaces)
using namespace std;

#endif//USE_OPENCV

CaffeClassifier::CaffeClassifier(const string& model_file,
               const string& trained_file, const string& preprocess_file) {
  	
  	/* Set caffe model to execute on cpu */

    Caffe::set_mode(Caffe::CPU);

    /* Load the network. */
    net_.reset(new Net<float>(model_file, TEST));
    net_->CopyTrainedLayersFrom(trained_file);

    CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
    CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

    Blob<float> *input_layer = net_->input_blobs()[0];
    num_channels_ = input_layer->channels();
    CHECK(num_channels_ == 3 || num_channels_ == 1)
            << "Input layer should have 1 or 3 channels.";

    /* Get network input layer shape */
    input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

    /* Load Preprocess Parameters*/
    LoadPreprocessParamters(preprocess_file);
}


/* Return the top N predictions. */
std::vector<float> CaffeClassifier::Classify(const cv::Mat& img, int N) {
    std::vector<float> output = Predict(img);

    return output;
}

std::vector<float> CaffeClassifier::Predict(const cv::Mat& img) {
    //Blob<float>* input_layer = net_->input_blobs()[0];
    //input_layer->Reshape(1, num_channels_,
    //                     input_geometry_.height, input_geometry_.width);
    /* Forward dimension change to all layers. */
    //net_->Reshape(); //NEEDED FOR DYNAMIC FULL CONV NETS!

    std::vector<cv::Mat> input_channels;
    WrapInputLayer(&input_channels);

    Preprocess(img, &input_channels);

    net_->Forward();

    /* Copy the output layer to a std::vector */
    Blob<float> *output_layer = net_->output_blobs()[0];
    const float *begin = output_layer->cpu_data();
    const float *end = begin + output_layer->channels();
    return std::vector<float>(begin, end);
}


/* Wrap the input layer of the network in separate cv::Mat objects
 * (one per channel). This way we save one memcpy operation and we
 * don't need to rely on cudaMemcpy2D. The last preprocessing
 * operation will write the separate channels directly to the input
 * layer. */
void CaffeClassifier::WrapInputLayer(std::vector<cv::Mat> *input_channels) {
    Blob<float> *input_layer = net_->input_blobs()[0];

    int width = input_layer->width();
    int height = input_layer->height();
    float *input_data = input_layer->mutable_cpu_data();
    for (int i = 0; i < input_layer->channels(); ++i) {
        cv::Mat channel(height, width, CV_32FC1, input_data);
        input_channels->push_back(channel);
        input_data += width * height;
    }
}

bool CaffeClassifier::PairCompare(const std::pair<float, int>& lhs,
                        const std::pair<float, int>& rhs){
    return lhs.first > rhs.first;
}

/* Return the indices of the top N values of vector v. */
std::vector<int> CaffeClassifier::Argmax(const std::vector<float>& v, int N) {
    std::vector<std::pair<float, int> > pairs;
    for (size_t i = 0; i < v.size(); ++i)
        pairs.push_back(std::make_pair(v[i], i));
    std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

    std::vector<int> result;
    for (int i = 0; i < N; ++i)
        result.push_back(pairs[i].second);
    return result;
}


void CaffeClassifier::Preprocess(const cv::Mat &img,
                                 std::vector<cv::Mat> *input_channels) {
    /* Convert the input image to the input image format of the network. */
    cv::Mat sample;
    /*
    if (img.channels() == 3 && num_channels_ == 1)
      cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
    else if (img.channels() == 4 && num_channels_ == 1)
      cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
    else if (img.channels() == 4 && num_channels_ == 3)
      cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
    else if (img.channels() == 1 && num_channels_ == 3)
      cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
    else
    */
    sample = img;

    //cv::Mat sample_resized;
    //std::cout << "sample size: " << sample.size() << std::endl;
    //std::cout << "input_geometry_ width: " << input_geometry_.width << ", input_geometry_ height: " << input_geometry_.height << std::endl;

    if (sample.size() != input_geometry_) {
        cv::resize(sample, sample, input_geometry_, 0, 0, cv::INTER_NEAREST);
    }

    //cv::Mat sample_float;

    sample.convertTo(sample, CV_32FC1);


    //cv::divide(255, sample_float, sample_float); //<- dont use this.. this is doing wiered stuff >.<
    sample = sample / 255.f;
    sample = sample - this->mean;
    sample = sample / this->stddev;

    cv::split(sample, *input_channels);

    CHECK(reinterpret_cast<float *>(input_channels->at(0).data)
          == net_->input_blobs()[0]->cpu_data())
            << "Input channels are not wrapping the input layer of the network.";
}

void CaffeClassifier::setMean(float mean){
    this->mean = mean;
}

void CaffeClassifier::setStddev(float stddev){
    this->stddev = stddev;
}

void CaffeClassifier::LoadPreprocessParamters(const string& preprocess_file){
    /*
    Load preprocessing parameters from file where first entry is the mean and second line is the stddev.
    Currently only for grayscale images.
    */
    cout << "Loading Preprocess From: " << preprocess_file << endl;
    ifstream file(preprocess_file);
    float _mean = 0.f;
    float _stddev = 1.f;
    
    file >> _mean;
    file >> _stddev;
    cout << "mean: " << _mean << endl;
    cout << "stddev: " << _stddev << endl;

    setMean(_mean);
    setStddev(_stddev);
}
