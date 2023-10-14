#pragma once
#include <memory>
#include <framework/image/camimage.h>
#include <framework/util/clock.h>
#include <framework/datastructures/mpsc_storage.h>
#include "visiondefinitions.h"
#include "visioncontext.h"

struct VisionImageBase {
    CamImage image;

    VisionImageBase()
        : image() {
        
    }

    VisionImageBase(CamImage &_image)
        : image(_image) {
    }

    CamImage & operator*() {
        return image;
    }
};

struct VisionImageProcessed : public VisionImageBase {
    std::vector<VisionResult> results;

    VisionImageProcessed() = default;

    VisionImageProcessed(CamImage &img)
        : VisionImageBase(img) {
            image.lock(ImgLock::WRITER);
            image.lock(ImgLock::DEBUG);
        }

    void release() {
        image.unlock(ImgLock::DEBUG);
    }
};

struct VisionImage : public VisionImageBase {
    int id;
    VisionContext context;
    MPSCStorage<std::vector<VisionResult>> results;

    VisionImage() = default;

    VisionImage(CamImage &_image)
        : VisionImageBase(_image) {
        image.lock(ImgLock::VISION);
    }

    ~VisionImage() {
        image.unlock(ImgLock::VISION);
    }
};
