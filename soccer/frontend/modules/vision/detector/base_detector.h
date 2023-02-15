#pragma once
#include <cstdint>

#include <framework/image/camimage.h>

#define CHECK_RANGE(coord,low,high)

namespace bbvision {

class BaseDetector {
protected:
    const int width;
    const int height;
    const int8_t *lutCb;
    const int8_t *lutCr;

    BaseDetector(const int _width, const int _height, const int8_t *_lutCb,
                 const int8_t *_lutCr) :
        width(_width), height(_height), lutCb(_lutCb), lutCr(_lutCr) {}

    inline uint8_t getY(const uint8_t *const img, int32_t x,
                        int32_t y) const __attribute__((nonnull)) __attribute__((pure)) {
        CHECK_RANGE(x, 0, width-1);
        CHECK_RANGE(y, 0, height-1);
        return img[(x + y * width) << 1];
    }

    inline uint8_t getCb(const uint8_t *const img, int32_t x,
                         int32_t y) const __attribute__((nonnull)) __attribute__((pure)) {
        CHECK_RANGE(x, 0, width-1);
        CHECK_RANGE(y, 0, height-1);
        return img[((x + y * width) << 1) + lutCb[x]];
    }

    inline uint8_t getCr(const uint8_t *const img, int32_t x,
                         int32_t y) const __attribute__((nonnull)) __attribute__((pure)) {
        CHECK_RANGE(x, 0, width-1);
        CHECK_RANGE(y, 0, height-1);
        return img[((x + y * width) << 1) + lutCr[x]];
    }

    inline void setY(uint8_t *const img, const int32_t x, int32_t y,
                     const uint8_t c) __attribute__((nonnull)) {
        CHECK_RANGE(x, 0, width-1);
        CHECK_RANGE(y, 0, height-1);
        img[(x+y*width)<<1]=c;
    }

};

} //namespace bbvision
