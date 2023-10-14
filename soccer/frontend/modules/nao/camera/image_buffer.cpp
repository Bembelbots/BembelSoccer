#include "image_buffer.h"
#include <representations/camera/camera.h>

#include <chrono>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <bitset>
#include <cstdlib>

#include <framework/logger/logger.h>
#include <representations/bembelbots/constants.h>
#include <iostream>

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

void ImageBuffer::initialize(const size_t &size, const int &cam, bool sim) {
    camName = (cam == TOP_CAMERA) ? "top" : "bottom";
    simulator = sim;
    data.resize(size);
    for (size_t i{0}; i < size; ++i) {
        // create new CamImage with aligned memory
        uint8_t *p;
        int ret = posix_memalign(reinterpret_cast<void **>(&p), 16, imgSize);
        assert(ret == 0); // if posix_memalign fails, all hell will break loose, so just give up
        data[i] = std::move(CamImage(camera::w, camera::h, cam));
        data[i].setData(p);
    }
}

void ImageBuffer::deinit() {
    camName = "";
    // delete all allocated memory
    for (CamImage &i : data) {
        if (i.data){
            free(i.data);
        }
        i.data = nullptr;
    }
    data.clear();
}

ImageBuffer::~ImageBuffer() {
    deinit();
}

CamImage &ImageBuffer::getImage() {
    uint32_t c{0}, limit{100};
    
    // basically sleep forever in simulator mode
    if (simulator)
        limit = UINT_MAX;
    
    while (lastImg == curImg && (++c < limit))
        sleep_for(1ms);
    LOG_ERROR_IF(c == limit) << __PRETTY_FUNCTION__ << ": failed to grab new " << camName << " image";
    lastImg = curImg.load();
    return data[curImg];
}

CamImage &ImageBuffer::getCaptureBuffer() {
    // find free buffer
    // check next position first
    uint32_t i = curImg;
    do {
        i = (i + 1) % data.size();
        if (!data[i].isLocked())
            break;
    } while (i != curImg);

    // no free buffer found, bail out!
    if (i == curImg)
        throw std::runtime_error("No free buffer to store new image!");

    capture = i;
    data[i].lock(ImgLock::CAPTURE);
    return data[i];
}

void ImageBuffer::releaseCaptureBuffer() {
    auto &img = data[capture];
    assert(img.hasLock(ImgLock::CAPTURE)); // getCaptureBuffer() has not been called first
    curImg = capture.load();
    img.unlock(ImgLock::CAPTURE);
}

// vim: set ts=4 sw=4 sts=4 expandtab:
