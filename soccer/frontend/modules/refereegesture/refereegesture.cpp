#include "refereegesture.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <tensorflow/lite/kernels/register.h>
#include <representations/bembelbots/constants.h>

#include <framework/thread/util.h>

void RefereeGesture::setup() {
    poseDetectionModel = tflite::FlatBufferModel::BuildFromFile(
            (settings->configPath + "../nn/movenet-tflite-singlepose-lightning-tflite-int8-v1.tflite").c_str());
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*poseDetectionModel, resolver)(&poseDetectionInterpreter);
    poseDetectionInterpreter->AllocateTensors();
}

void RefereeGesture::connect(rt::Linker &link) {
    link.name = "RefereeGesture";
    link(settings);
    link(inputImage);
    link(inputGameControl);
    link(event);
    link(debug);
}

void RefereeGesture::process() {
    if (inputGameControl->gameState != bbapi::GameState::STANDBY) {
        sleep_for(1ms);
        return;
    }

    VisionImageProcessed img;
    auto fetchedImages = inputImage.fetch();
    bool found = false;
    for (int i = fetchedImages.size(); i > 0; --i) {
        if (fetchedImages[i - 1].camera == TOP_CAMERA) {
            img = fetchedImages[i - 1];
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }

    cv::Mat converted = cv::imdecode(*img.jpeg, cv::IMREAD_COLOR);

    //cv::cvtColor(img.mat(), converted, cv::COLOR_YUV2RGB_YUY2); // This conversion would we used if we had the original image mat instead of the JPEG data

    // Cut relevant frame of the image
    converted = converted(cv::Range(100, 100 + 256), cv::Range(192, 640 - 192));

    cv::resize(converted, converted, cv::Size(192, 192));

    uint8_t *input_tensor = poseDetectionInterpreter->typed_input_tensor<uint8_t>(0);
    std::memcpy(input_tensor, converted.data, 192 * 192 * 3 * sizeof(uint8_t));

    jsassert(poseDetectionInterpreter->Invoke() == kTfLiteOk) << "Failed to run pose detection";

    // According to the docs:
    // The first two channels of the last dimension represents the yx coordinates (normalized to image frame, i.e. range in [0.0, 1.0]) of the 17 keypoints (in the order of: [nose, left eye, right eye, left ear, right ear, left shoulder, right shoulder, left elbow, right elbow, left wrist, right wrist, left hip, right hip, left knee, right knee, left ankle, right ankle]).
    // The third channel of the last dimension represents the prediction confidence scores of each keypoint, also in the range [0.0, 1.0].
    constexpr size_t POS_LEFT_SHOULDER = 5;
    constexpr size_t POS_RIGHT_SHOULDER = 6;
    constexpr size_t POS_LEFT_ELBOW = 7;
    constexpr size_t POS_RIGHT_ELBOW = 8;
    constexpr size_t POS_LEFT_WRIST = 9;
    constexpr size_t POS_RIGHT_WRIST = 10;
    const float *output_tensor = poseDetectionInterpreter->typed_output_tensor<float>(0);

    auto x = [output_tensor](const size_t pos) { return output_tensor[pos * 3 + 1]; };
    auto y = [output_tensor](const size_t pos) { return output_tensor[pos * 3]; };
    auto conf = [output_tensor](const size_t pos) { return output_tensor[pos * 3 + 2]; };

    // TODO: Look at confidence scores.
    const bool left_arm_up = y(POS_LEFT_WRIST) < y(POS_LEFT_ELBOW) && y(POS_LEFT_ELBOW) < y(POS_LEFT_SHOULDER);
    const bool right_arm_up = y(POS_RIGHT_WRIST) < y(POS_RIGHT_ELBOW) && y(POS_RIGHT_ELBOW) < y(POS_RIGHT_SHOULDER);

    for (size_t element = POS_LEFT_SHOULDER; element <= POS_RIGHT_WRIST; ++element) {
        const auto center = cv::Point(192 * x(element), 192 * y(element));
        cv::circle(converted, center, 2, cv::Scalar(0, conf(element) * 255, 255), cv::FILLED, cv::LINE_8);
    }

    constexpr std::array<std::pair<size_t, size_t>, 4> lines{
            std::make_pair(POS_LEFT_SHOULDER, POS_LEFT_ELBOW),
            std::make_pair(POS_LEFT_ELBOW, POS_LEFT_WRIST),
            std::make_pair(POS_RIGHT_SHOULDER, POS_RIGHT_ELBOW),
            std::make_pair(POS_RIGHT_ELBOW, POS_RIGHT_WRIST),
    };
    for (const auto &line : lines) {
        const auto start = cv::Point(192 * x(line.first), 192 * y(line.first));
        const auto end = cv::Point(192 * x(line.second), 192 * y(line.second));
        cv::line(converted, start, end, cv::Scalar(255, 0, 0), 1, cv::LINE_8);
    }

    if (left_arm_up and right_arm_up) {
        LOG_SAY << "Saw referee!";
    }

    bbapi::RefereeGestureMessageT gesture;
    gesture.leftArmUp = left_arm_up;
    gesture.rightArmUp = right_arm_up;
    event.emit(gesture);
    debug.emit({converted});
}
