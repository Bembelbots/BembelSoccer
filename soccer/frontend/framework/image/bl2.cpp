#include "bl2.h"

#include "camimage.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <framework/util/assert.h>



CamImage Bl2::load(const std::string &filename, size_t w, size_t h, int camera) {
    //std::cout << "loading image " << filename << std::endl;
    size_t result;
    size_t size = w * h * 2;

    uint8_t *YUV422 = nullptr;

    int r = posix_memalign(reinterpret_cast<void **>(&YUV422), 8, sizeof(uint8_t)*size);
    jsassert(r == 0);

    if (YUV422 == nullptr) {
        std::cout << "decoder error: failed to allocate mem" << std::endl;
        return CamImage();
    }

    FILE *pFile;
    pFile = fopen(filename.c_str(), "rb");

    if (!pFile) {
        std::cout << "decoder error: can't open file!" << std::endl;
        free(YUV422);
        YUV422 = nullptr;
        return CamImage();
    }

    fseek(pFile, 0, SEEK_END);
    size_t imageFileSize = ftell(pFile);
    rewind(pFile);

    result = fread(YUV422, sizeof(uint8_t), size, pFile);

    if (result != size) {
        std::cout << "decoder error: read size != image size!" << std::endl;
        free(YUV422);
        YUV422 = nullptr;
        fclose(pFile);
        return CamImage();
    }

    size_t readBytes = size;

    CamImage img(YUV422, w, h, camera);

    if (imageFileSize != readBytes) {
        //@TODO load all required data for the transform using log data
        std::cout << "camera transformation included in this image" << std::endl;

        camPose eulers;

        std::cout << "eulers: ";
        for (size_t i = 0; i < 3; ++i) {
            float f;
            fread(&f, sizeof(float), 1, pFile);
            eulers.v[i] = f;
            readBytes += sizeof(float);
            std::cout << f << ", ";
        }
        for (size_t i = 0; i < 3; ++i) {
            float f;
            fread(&f, sizeof(float), 1, pFile);
            eulers.r[i] = f;
            readBytes += sizeof(float);
            std::cout << f << ", ";
        }
        std::cout << std::endl;

        if (imageFileSize != readBytes) {
            std::cout << "principle points included in this image" << std::endl;
            float principlePointX;
            fread(&principlePointX, sizeof(float), 1, pFile);
            std::cout << "PPX: " << principlePointX << std::endl;

            float principlePointY;
            fread(&principlePointY, sizeof(float), 1, pFile);
            std::cout << "PPY: " << principlePointY << std::endl;

            img.setCalibration(principlePointX, principlePointY, 597.1f, 587.3f);
            img.setTransform(eulers);
        } else {
            // 0.0 is bad here. guessing a point in the middle is better
            img.setCalibration(img.width / 2.f, img.height / 2.f, 597.1f, 587.3f);
            img.setTransform(eulers);
        }
    }

    fclose(pFile);
    return img;
}
