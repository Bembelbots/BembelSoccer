#include "image.h"
#include <iostream>

#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

ImageBase::ImageBase() :
    width(0),
    height(0),
    data(nullptr),
    timestamp(0),
    locks(new ImgLocks_t),
    channels(0),
    cvType(-1) {
}

ImageBase::~ImageBase() {
}

ImageBase::ImageBase(uint8_t *img, size_t width, size_t height, int channels,
                     int cvType) :
    width(width),
    height(height),
    data(img),
    timestamp(0),
    locks(new ImgLocks_t),
    p_container(std::shared_ptr<uint8_t>(img, deleteImagePointer)),
    channels(channels),
    cvType(cvType) {
}

ImageBase::ImageBase(size_t width, size_t height, int channels, int cvType) :
    width(width),
    height(height),
    data(nullptr),
    timestamp(0),
    locks(new ImgLocks_t),
    channels(channels),
    cvType(cvType) {
}

IplImage *ImageBase::getIplHeader() const {
    return getIplHeader(width, height);
}

IplImage *ImageBase::getIplHeader(size_t w, size_t h) const {
    return cvCreateImageHeader(cvSize(w, h), 8, channels);
}

IplImage *ImageBase::getIplImage() const {
    IplImage *ipl = getIplHeader();
    ipl->imageData = reinterpret_cast<char *>(data);
    return ipl;
}

void ImageBase::deleteImagePointer(uint8_t *img) {
    free(img); // posix_memalign needs free
}

bool ImageBase::hasData() {
    return data != nullptr || p_container.use_count() > 0;
}

cv::Mat ImageBase::mat() const {
    return cv::Mat(height, width, cvType, data);
}

bool ImageBase::isValid() {
    return hasData();
}

void ImageBase::resize(size_t w, size_t h) {
    cv::Mat imgMat = mat();

    cv::resize(imgMat, imgMat, cv::Size(), w / float(width), h / float(height), cv::INTER_NEAREST);

    width = w;
    height = h;
}

bool ImageBase::isInside(size_t w, size_t h) const {
    return w <= width && h <= height;
}

void ImageBase::setData(uint8_t *img) {
    if (width == 0 || height == 0) {
        return;
    }

    data = img;
}

void ImageBase::setData(char *data) {
    setData(reinterpret_cast<uint8_t *>(data));
}

void ImageBase::write(std::ofstream &file) {
    file.write(reinterpret_cast<char *>(data), width*height*channels);
}

void ImageBase::saveAs(std::string path) {
    cv::imwrite(path.c_str(), mat());
}

bool ImageBase::isLocked() const {
    auto l = locks.get()->load();
    return l.any();
}

bool ImageBase::hasLock(const ImgLock &lt) const {
    auto l = locks.get()->load();
    return l[int(lt)];
}

void ImageBase::lock(const ImgLock &lt) {
    auto l = locks.get()->load();
    l.set(int(lt));
    locks.get()->store(l);
}

void ImageBase::unlock(const ImgLock &lt) {
    auto l = locks.get()->load();
    l.reset(int(lt));
    locks.get()->store(l);
}



// vim: set ts=4 sw=4 sts=4 expandtab:
