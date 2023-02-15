#pragma once

#include "image.h"

class RgbImage;

class YuvPixel : Pixel {
public:
    int Y;
    int U;
    int V;

    YuvPixel() : Y(0), U(0), V(0) {};

    YuvPixel(int Y, int U, int V)
        : Y(Y), U(U), V(V) {};

    bool operator == (const YuvPixel &p) const {
        return Y == p.Y && U == p.U && V == p.V;
    }
};

class YuvImage : public ImageBase {
public:
    static constexpr int channels = 2; // cppcheck-suppress duplInheritedMember
    static double *msvLUT;

    enum YuvChannel {
        Y,
        U,
        V
    };

    YuvImage() : ImageBase() {};

    YuvImage(uint8_t *data, size_t width, size_t height)
        : ImageBase(data, width, height, channels, CV_8UC2) {
        setData(data);
        createMsvLUT();
        createLUT(width);
    };

    YuvImage(size_t width, size_t height)
        : ImageBase(width, height, channels, CV_8UC2) {
        createMsvLUT();
        createLUT(width);
    };

    virtual ~YuvImage();
    void normalize();

    YuvImage getImage();

    RgbImage toRGB(int newWidth = 0, int newHeight = 0) const;

    cv::Mat channel(YuvChannel channel) const;

    RgbImage hist();

    void setY(const size_t &x, const size_t &y, const uint8_t &c);
    void setYUV(const size_t &x, const size_t &y, const YuvPixel &p);
    void setRGB(const size_t &x, const size_t &y, const uint8_t &r,
                const uint8_t &g, const uint8_t &b);

    inline uint8_t getY(size_t x, size_t y) const {
        return data[(x + y * width) << 1];
    }

    inline uint8_t getU(size_t x, size_t y) const {
        return data[((x + y * width) << 1) + lutU[x]];
    }

    inline uint8_t getV(size_t x, size_t y) const {
        return data[((x + y * width) << 1) + lutV[x]];
    }

    inline YuvPixel getYUV(size_t x, size_t y) {
        return YuvPixel(data[(x + y * width) << 1],
                        data[((x + y * width) << 1) + lutU[x]],
                        data[((x + y * width) << 1) + lutV[x]]);
    }

protected:
    static int8_t *lutU;
    static int8_t *lutV;

    static void createMsvLUT();
    static void createLUT(size_t width, bool recreate = false);
};
