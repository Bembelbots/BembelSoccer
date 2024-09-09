#pragma once
#include <memory>
#include "sources/jsvideosource.h"

struct NaoCameras {
    std::unique_ptr<JsVideoSource> top;
    std::unique_ptr<JsVideoSource> bottom;

    explicit NaoCameras(bool simulator);

    void openCamera(const int &cam, std::string simulatorHost = "", bool docker = false);
    bool initialized() const;
private:
    bool simulator{false};
};
