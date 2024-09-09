#pragma once

#include "representations/flatbuffers/types/actuators.h"
#include <deque>
#include <unordered_map>

#include <bodycontrol/internals/submodule.h>
#include <representations/motion/body_commands.h>

class LedControl : public SubModule {
public:
    LedControl() {}

    void setup(Setup s) override { s.cmds->connect<SetLeds, &LedControl::handleRequest>(this); }

    SubModuleReturnValue step(BodyBlackboard *bb) override {
        bb->actuators->led = leds;
        return RUNNING;
    }

    /**
     * Handle commands
     */
    void handleRequest(SetLeds req) {
        Color color(req.color);
        switch (req.group) {
            case LED::CHEST:
                updateSingle(leds.chest, color);
                break;

            case LED::LEFT_EAR:
                updateEar(leds.ears.left, req.color);
                break;

            case LED::RIGHT_EAR:
                updateEar(leds.ears.right, req.color);
                break;

            case LED::LEFT_EYE:
                updateEye(leds.eyes.left, LEDSide::All, color);
                break;

            case LED::LEFT_EYE_LEFT:
                updateEye(leds.eyes.left, LEDSide::Left, color);
                break;

            case LED::LEFT_EYE_RIGHT:
                updateEye(leds.eyes.left, LEDSide::Right, color);
                break;

            case LED::RIGHT_EYE:
                updateEye(leds.eyes.right, LEDSide::All, color);
                break;

            case LED::RIGHT_EYE_LEFT:
                updateEye(leds.eyes.right, LEDSide::Left, color);
                break;

            case LED::RIGHT_EYE_RIGHT:
                updateEye(leds.eyes.right, LEDSide::Right, color);
                break;

            case LED::LEFT_FOOT:
                updateSingle(leds.feet.left, color);
                break;

            case LED::RIGHT_FOOT:
                updateSingle(leds.feet.right, color);
                break;

            case LED::SKULL:
                updateSkull(leds.skull, LEDSide::All, color.brightness);
                break;

            case LED::SKULL_LEFT:
                updateSkull(leds.skull, LEDSide::Left, color.brightness);
                break;

            case LED::SKULL_RIGHT:
                updateSkull(leds.skull, LEDSide::Right, color.brightness);
                break;

            case LED::ALL:
                updateSingle(leds.chest, color);
                updateEye(leds.eyes.left, LEDSide::All, color);
                updateEye(leds.eyes.right, LEDSide::All, color);
                updateSingle(leds.feet.left, color);
                updateSingle(leds.feet.right, color);
                updateSkull(leds.skull, LEDSide::All, color.brightness);
                break;
        }
    }

private:
    bbipc::LED leds;
    enum class LEDSide { Left, Right, All };

    struct Color {
        float r, g, b;
        float brightness;

        // convert RGB integer to separate floats
        explicit Color(const int &c) : brightness(c / 255.f) {
            r = ((c >> 16) & 0xFF) / 255.f;
            g = ((c >> 8) & 0xFF) / 255.f;
            b = (c & 0xFF) / 255.f;
        }
    };

    void updateSingle(bbipc::LEDSingle &led, const Color &color) {
        led.r = color.r;
        led.g = color.g;
        led.b = color.b;
    }

    template<size_t N>
    void updateEye(bbipc::LEDString<N> &eye, const LEDSide side, const Color &color) {
        size_t start{0}, end{N / 2};
        switch (side) {
            case LEDSide::Left:
                break;
            case LEDSide::Right:
                start = end;
                break;
            case LEDSide::All:
                end = eye.NUM_LEDS;
                break;
        }
        for (size_t i{start}; i < end; ++i) {
            eye.r[i] = color.r;
            eye.g[i] = color.g;
            eye.b[i] = color.b;
        }
    }

    void updateEar(bbipc::LEDEar &ear, size_t range) {
        LOG_WARN << __PRETTY_FUNCTION__ << " - setting ear LEDs from frontend is not supported";
    }

    void updateSkull(bbipc::LEDSkull &skull, const LEDSide side, const float value) {
        size_t start{0}, end{skull.size() / 2};
        switch (side) {
            case LEDSide::Left:
                break;
            case LEDSide::Right:
                start = end;
                break;
            case LEDSide::All:
                end = skull.size();
                break;
        }

        for (size_t i{start}; i < end; ++i)
            skull[i] = value;
    }
};
