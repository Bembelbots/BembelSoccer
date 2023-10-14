#include "yuv422.h"

#include "rgb.h"

// CV_LOAD_IMAGE_COLOR
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

#include <representations/camera/camera.h>

IplImage *convertYuv422ToYCrCb(IplImage *yuv422) {
    IplImage *yuv = cvCreateImage(cvSize(yuv422->width, yuv422->height), 8, 3);
    for (int x = 0; x < yuv422->width; ++x) {
        for (int y = 0; y < yuv422->height; ++y) {
            int basex = (x / 2) * 4; // round, and *2
            int basey = yuv422->widthStep * y;
            uchar r = static_cast<uchar>((reinterpret_cast<uchar*>(yuv422->imageData + basey))[basex + 2
                                               * (x % 2)]);
            uchar g = static_cast<uchar>((reinterpret_cast<uchar*>(yuv422->imageData + basey))[basex +
                                               1]);
            uchar b = static_cast<uchar>((reinterpret_cast<uchar*>(yuv422->imageData + basey))[basex +
                                               3]);

            (reinterpret_cast<uchar*>(yuv->imageData + yuv->widthStep * y))[x * 3] = r;
            (reinterpret_cast<uchar*>(yuv->imageData + yuv->widthStep * y))[x * 3 + 1] = g;
            (reinterpret_cast<uchar*>(yuv->imageData + yuv->widthStep * y))[x * 3 + 2] = b;
        }
    }
    return yuv;
}


YuvImage::~YuvImage() {
    if (msvLUT != nullptr) {
        delete[] msvLUT;
        msvLUT = nullptr;
    }

    if (lutU != nullptr) {
        delete[] lutU;
        lutU = nullptr;
    }

    if (lutV != nullptr) {
        delete[] lutV;
        lutV = nullptr;
    }
}
void convertYuv422ToRgb(IplImage *yuv422, IplImage *target) {
    IplImage *yuv = convertYuv422ToYCrCb(yuv422);

    // convert image to RGB
    cvCvtColor(yuv, target, CV_YCrCb2RGB);

    // release tmp image
    cvReleaseImage(&yuv);
}

void YuvImage::setYUV(const size_t &x, const size_t &y, const YuvPixel &p) {
    data[(x+y*width)<<1]=p.Y;
    data[((x+y*width)<<1) + lutU[x]]=p.U;
    data[((x+y*width)<<1) + lutV[x]]=p.V;
}

void YuvImage::setY(const size_t &x, const size_t &y, const uint8_t &c) {
    data[(x+y*width)<<1]=c;
}

void YuvImage::setRGB(const size_t &x, const size_t &y, const uint8_t &r,
                      const uint8_t &g, const uint8_t &b) {
    data[x * 2 + y * width * 2] = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
    if (x % 2 == 0) {
        data[1 + x * 2 + y * width * 2] =
            (uint8_t)(-0.169 * r - 0.331 * g + 0.499 * b + 128);
    } else {
        data[1 + x * 2 + y * width * 2] =
            (uint8_t)(0.498 * r - 0.419 * g - 0.0813 * b + 128);
    }
}

void YuvImage::normalize() {
    std::vector<int> cache(256);

    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            uint8_t Y = getY(x, y);
            for (size_t i = Y; i < cache.size(); i++) {
                cache[i] += Y;
            }
        }
    }

    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            setY(x, y, cache[getY(x, y)] / (width * height));
        }
    }
}

YuvImage YuvImage::getImage() {
    YuvImage img = YuvImage(width, height);
    img.setData(data);
    return img;
}

RgbImage YuvImage::toRGB(int newWidth, int newHeight) const {
    cv::Mat imgMat = mat();

    cv::Mat dest;
    cv::cvtColor(imgMat, dest, CV_YUV2RGB_YVYU);

    if(newWidth && newHeight) {
        cv::resize(dest, dest, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_NEAREST);
    }

    return RgbImage(dest);
}

cv::Mat YuvImage::channel(YuvChannel channel) const {
    cv::Mat dest(height, width, CV_8UC1);

    const int cols = dest.cols;
    const int step = dest.channels();
    //const int rows = dest.rows;

    for (size_t y = 0; y < height; y++) {

        unsigned char *row = dest.ptr(y);
        unsigned char *row_end = row + cols*step;

        for (size_t x = 0; row != row_end; row += step, x++) {

            uint8_t color = {};

            switch (channel) {
            case Y:
                color = getY(x, y);
                break;
            case U:
                color = getU(x, y);
                break;
            case V:
                color = getV(x, y);
                break;
            }

            unsigned char *c1 = row;

            *c1 = color;
        }
    }

    return dest;
}

RgbImage YuvImage::hist() {
    IplImage *yuv422 = getIplImage();
    cv::Mat ycrcb(cv::cvarrToMat(convertYuv422ToYCrCb(yuv422)));

    /// Separate the image in 3 places
    std::vector<cv::Mat> planes;
    cv::split(ycrcb, planes);

    /// Establish the number of bins
    int histSize = 256;

    /// Set the ranges ( for Y Cr Cb) )
    float range[] = { 0, 256 } ;
    const float *histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    cv::Mat y_hist, cr_hist, cb_hist;

    /// Compute the histograms:
    cv::calcHist(&planes[0], 1, 0, cv::Mat(), y_hist, 1, &histSize, &histRange,
                 uniform, accumulate);
    cv::calcHist(&planes[1], 1, 0, cv::Mat(), cr_hist, 1, &histSize, &histRange,
                 uniform, accumulate);
    cv::calcHist(&planes[2], 1, 0, cv::Mat(), cb_hist, 1, &histSize, &histRange,
                 uniform, accumulate);

    // Draw the histograms for Y, Cr and Cb
    int hist_w = camera::w;
    int hist_h = camera::h;
    int bin_w = cvRound((double) hist_w/histSize);

    cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0));

    /// Normalize the result to [ 0, histImage.rows ]
    cv::normalize(y_hist, y_hist, 0, histImage.rows, cv::NORM_MINMAX, -1,
                  cv::Mat());
    cv::normalize(cr_hist, cr_hist, 0, histImage.rows, cv::NORM_MINMAX, -1,
                  cv::Mat());
    cv::normalize(cb_hist, cb_hist, 0, histImage.rows, cv::NORM_MINMAX, -1,
                  cv::Mat());

    /// Draw for each channel
    for (int i = 1; i < histSize; i++) {
        line(histImage, cv::Point(bin_w*(i-1),
                                  hist_h - cvRound(y_hist.at<float>(i-1))),
             cv::Point(bin_w*(i), hist_h - cvRound(y_hist.at<float>(i))),
             cv::Scalar(255, 255, 255), 2, 8, 0);
        line(histImage, cv::Point(bin_w*(i-1),
                                  hist_h - cvRound(cb_hist.at<float>(i-1))),
             cv::Point(bin_w*(i), hist_h - cvRound(cb_hist.at<float>(i))),
             cv::Scalar(255, 0, 0), 2, 8, 0);
        line(histImage, cv::Point(bin_w*(i-1),
                                  hist_h - cvRound(cr_hist.at<float>(i-1))),
             cv::Point(bin_w*(i), hist_h - cvRound(cr_hist.at<float>(i))),
             cv::Scalar(0, 0, 255), 2, 8, 0);
    }

    cvReleaseImage(&yuv422);

    return RgbImage(histImage);
}

// what are they doing here? somewhere in the file, class members???
// \TODO: remove this fuck shit
int8_t *YuvImage::lutU = nullptr;
int8_t *YuvImage::lutV = nullptr;
double *YuvImage::msvLUT = nullptr;

void YuvImage::createMsvLUT() {
    if (msvLUT != nullptr) {
        return;
    }

    msvLUT = new double[sizeof(*msvLUT)*256];

    const int pivot = 255 / 4; // 5 bins

    for (size_t i = 0; i <= 255; i++) {
        msvLUT[i] = (int(i / pivot) + 1) * i; // (j + 1) * xj
    }
}

void YuvImage::createLUT(size_t width, bool recreate) {
    if (!recreate && (lutU != nullptr || lutV != nullptr || width == 0)) {
        return;
    }

    if (lutU != nullptr) {
        delete[] lutU;
        lutU = nullptr;
    }

    if (lutV != nullptr) {
        delete[] lutV;
        lutV = nullptr;
    }

    //@todo allow for images of different sizes to be loaded

    lutU = new int8_t[sizeof(*lutU)*width];
    lutV = new int8_t[sizeof(*lutV)*width];

    for (size_t i = 1; i < width; i++) {
        if ((i&1) == 0) {
            lutU[i]=5;
            lutV[i]=3;
            continue;
        }

        lutU[i]=3;
        lutV[i]=5;

        while (lutV[i] + i*2 >= width*2) {
            lutV[i]-=4;
        }

        while (lutU[i] + i*2 >= width*2) {
            lutU[i]-=4;
        }
    }

    lutU[0]=1;
    lutV[0]=3;
}
