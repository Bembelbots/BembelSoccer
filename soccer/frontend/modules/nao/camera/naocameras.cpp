#include "naocameras.h"
#include <framework/logger/logger.h>
#include <representations/bembelbots/constants.h>

#include "framework/util/assert.h"
#include "sources/v6videosource.h"
#include "sources/tcpvideosource.h"

NaoCameras::NaoCameras(bool simulator)
    : simulator(simulator){
}

bool NaoCameras::initialized() const {
    return top && bottom;
}

void NaoCameras::openCamera(const int &cam, std::string simulatorHost, bool docker) {
    std::string camName = (cam == TOP_CAMERA) ? "top" : "bottom";
    LOG_INFO << "Opening " << camName << " camera";
    std::unique_ptr<JsVideoSource> camPtr;

    if (!simulator) {
        camPtr = std::make_unique<V6VideoSource>(cam);
    } else {
        camPtr = std::make_unique<TCPVideoSource>(cam, simulatorHost, docker);
        LOG_WARN << "using TCP camera source.";
    }

    camPtr->startCapturing();

    if(cam == TOP_CAMERA) {
        top = std::move(camPtr);
    } else {
        bottom = std::move(camPtr);
    }
}
