#include "rgb.h"
#include <framework/util/assert.h>

RgbImage::RgbImage(IplImage *ipl)
    :  ImageBase(ipl->width, ipl->height, channels, CV_8UC3) {

    // @TODO: Find better method copy data boese!!!
    size_t size = width*height*channels;

    //uint8_t *img = new uint8_t[size];
    uint8_t *img = nullptr;
    int ret = posix_memalign(reinterpret_cast<void **>(&img), 8, sizeof(uint8_t)*size);
    jsassert(ret == 0) << "posix_memalign() failed";
    memcpy(img, ipl->imageData, size);

    cvReleaseImage(&ipl);

    setData(img);
    p_container = std::shared_ptr<uint8_t>(data);
}

RgbImage::RgbImage(cv::Mat mat)
    :  ImageBase(mat.cols, mat.rows, channels, CV_8UC3) {
    setData(mat.data);
    m_container = mat;
}
