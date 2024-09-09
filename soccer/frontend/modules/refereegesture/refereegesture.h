#pragma once

#include "framework/rt/flags.h"
#include <framework/rt/module.h>

#include <memory>
#include <representations/vision/image.h>
#include <representations/blackboards/settings.h>
#include <referee_gesture_message_generated.h>
#include <gamecontrol_generated.h>

#include <tensorflow/lite/model.h>
#include <tensorflow/lite/interpreter.h>

struct RefereeGestureDebug {
    cv::Mat img;
};

class RefereeGesture : public rt::Module {
public:
    void setup() override;
    void connect(rt::Linker &) override;
    void process() override;

private:
    rt::Context<SettingsBlackboard> settings;
    /// TODO: Don't encode to JPEG and back
    rt::Input<VisionImageProcessed, rt::Snoop> inputImage;
    rt::Input<bbapi::GamecontrolMessageT> inputGameControl;
    rt::Output<bbapi::RefereeGestureMessageT, rt::Event> event;
    rt::Output<RefereeGestureDebug, rt::Event> debug;

    std::unique_ptr<tflite::FlatBufferModel> poseDetectionModel;
    std::unique_ptr<tflite::Interpreter> poseDetectionInterpreter;
};
