#include "colorclasses.h"

YuvPixel ColorClass::white = YuvPixel(200, 127, 127);
YuvPixel ColorClass::green = YuvPixel(149, 43, 21);

//hilfsfunktion: Winkel zwischen 3D koordinaten
inline float angle3D(float ax, float ay, float az, float bx, float by,
                     float bz) {
    float dotProduct = ax*bx + ay*by + az*bz;
    float cx = ay*bz-az*by;
    float cy = az*bx-ax*bz;
    float cz = ax*by-ay*bx;
    float crossProductLength = hypotf(cx, hypotf(cy, cz));
    return atan2f(crossProductLength, dotProduct);
}

ColorClass::Color ColorClass::GetClass(YuvPixel pixel) {

    if (pixel.Y >= 200) {
        return White;
    }

    if (hypotf(pixel.U-128, pixel.V-128) < 15) {
        return Black;
    }

    return Green;
}

YuvImage ColorClass::Classify(YuvImage img) {
    const int step = 4;

    uint32_t width = img.width / step;
    uint32_t height = img.height / step;

    uint8_t *data = new uint8_t[width*height*YuvImage::channels];

    YuvImage yuv422(data, width, height);

    LOOP_IMAGE(yuv422, 1)
    YuvPixel current = img.getYUV(x*step, y*step);
    ColorClass::Color c = GetClass(current);

    YuvPixel p(60, 202, 222);

    switch (c) {
    case Green:
        p.Y = 60;
        p.U = 43;
        p.V = 21;
        break;
    case White:
        p.Y = 255;
        p.U = 128;
        p.V = 128;
        break;
    case Black:
        p.Y = 0;
        p.U = 128;
        p.V = 128;
        break;
    case Grey:
        p.Y = current.Y;
        p.U = 128;
        p.V = 128;
        break;
    case Unknown:
        break;
    }

    yuv422.setYUV(x, y, p);
    LOOP_IMAGE_END(yuv422)

    return yuv422;
}
