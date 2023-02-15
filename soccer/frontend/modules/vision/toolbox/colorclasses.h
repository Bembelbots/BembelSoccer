#pragma once
#include <framework/image/yuv422.h>

class ColorClass {
public:
    enum Color {
        Green,
        White,
        Black,
        Grey,
        Unknown
    };

    static YuvPixel white;
    static YuvPixel green;

    static Color GetClass(YuvPixel pixel);
    static YuvImage Classify(YuvImage img);
};
