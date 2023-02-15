#pragma once

#include "image.h"

class RgbImage : public ImageBase {
public:
    static constexpr int channels = 3; // cppcheck-suppress duplInheritedMember

    enum RgbChannel {
        R,
        G,
        B
    };

    RgbImage(uint8_t *data, size_t width, size_t height)
        : ImageBase(data, width, height, channels, CV_8UC3) {
        setData(data);
    };

    RgbImage(size_t width, size_t height)
        : ImageBase(width, height, channels, CV_8UC3) {};

    explicit RgbImage(IplImage *ipl);

    explicit RgbImage(cv::Mat mat);
private:
    cv::Mat m_container;
};
