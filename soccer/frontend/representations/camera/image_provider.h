#pragma once

#include <modules/nao/camera/image_buffer.h>
#include "../bembelbots/constants.h"

class ImageProvider
{
public:
    ImageBuffer top;
    ImageBuffer bottom;

    CamImage getImage(const int &camera) {
        if (TOP_CAMERA == camera)
            return top.getImage();
        else
            return bottom.getImage();
    }
};