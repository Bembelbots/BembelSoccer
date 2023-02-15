#pragma once

#include <unordered_map>

#include <bodycontrol/internals/submodule.h>
#include <representations/motion/body_commands.h>

class LedControl : public SubModule {
public:
    LedControl() {
        for (const auto &g : enumValues<LED>)
            _internalState[g] = 0xFF000000;
    }

    void setup(Setup s) override { s.cmds->connect<SetLeds, &LedControl::handleRequest>(this); }

    SubModuleReturnValue step(BodyBlackboard *bb) override {
        float *actuators = bb->actuators.get().data();

        for (const auto &i : _requestedLeds) {
            LED group{i.first};
            int color{i.second};
            int r{(color >> 16) & 0xFF}, g{(color >> 8) & 0xFF}, b{color & 0xFF};

            switch (group) {
                case LED::LEFT_EYE_RIGHT:
                    actuatorUpdateRange(actuators, faceLedRedLeft0DegActuator, faceLedRedLeft135DegActuator, r);
                    actuatorUpdateRange(actuators, faceLedGreenLeft0DegActuator, faceLedGreenLeft135DegActuator, g);
                    actuatorUpdateRange(actuators, faceLedBlueLeft0DegActuator, faceLedBlueLeft135DegActuator, b);
                    break;

                case LED::LEFT_EYE_LEFT:
                    actuatorUpdateRange(actuators, faceLedRedLeft180DegActuator, faceLedRedLeft315DegActuator, r);
                    actuatorUpdateRange(actuators, faceLedGreenLeft180DegActuator, faceLedGreenLeft315DegActuator, g);
                    actuatorUpdateRange(actuators, faceLedBlueLeft180DegActuator, faceLedBlueLeft315DegActuator, b);
                    break;

                case LED::RIGHT_EYE_RIGHT:
                    actuatorUpdateRange(actuators, faceLedRedRight0DegActuator, faceLedRedRight135DegActuator, r);
                    actuatorUpdateRange(actuators, faceLedGreenRight0DegActuator, faceLedGreenRight135DegActuator, g);
                    actuatorUpdateRange(actuators, faceLedBlueRight0DegActuator, faceLedBlueRight135DegActuator, b);
                    break;

                case LED::RIGHT_EYE_LEFT:
                    actuatorUpdateRange(actuators, faceLedRedRight180DegActuator, faceLedRedRight315DegActuator, r);
                    actuatorUpdateRange(actuators, faceLedGreenRight180DegActuator, faceLedGreenRight315DegActuator, g);
                    actuatorUpdateRange(actuators, faceLedBlueRight180DegActuator, faceLedBlueRight315DegActuator, b);
                    break;

                case LED::CHEST:
                    actuators[chestBoardLedRedActuator] = r;
                    actuators[chestBoardLedGreenActuator] = g;
                    actuators[chestBoardLedBlueActuator] = b;
                    break;

                case LED::LEFT_FOOT:
                    actuators[lFootLedRedActuator] = r;
                    actuators[lFootLedGreenActuator] = g;
                    actuators[lFootLedBlueActuator] = b;
                    break;

                case LED::RIGHT_FOOT:
                    actuators[rFootLedRedActuator] = r;
                    actuators[rFootLedGreenActuator] = g;
                    actuators[rFootLedBlueActuator] = b;
                    break;

                case LED::LEFT_EAR:
                    updateEar(actuators, earsLedLeft0DegActuator, color);
                    break;

                case LED::RIGHT_EAR:
                    updateEar(actuators, earsLedRight0DegActuator, color);
                    break;

                case LED::BRAIN_LEFT:
                    updateBrain(actuators,
                            {headLedFrontLeft1Actuator,
                                    headLedFrontLeft0Actuator,
                                    headLedMiddleLeft0Actuator,
                                    headLedRearLeft0Actuator,
                                    headLedRearLeft1Actuator,
                                    headLedRearLeft2Actuator},
                            color);
                    break;

                case LED::BRAIN_RIGHT:
                    updateBrain(actuators,
                            {headLedFrontRight1Actuator,
                                    headLedFrontRight0Actuator,
                                    headLedMiddleRight0Actuator,
                                    headLedRearRight0Actuator,
                                    headLedRearRight1Actuator,
                                    headLedRearRight2Actuator},
                            color);
                    break;

                case LED::BRAIN:
                    updateBrain(actuators,
                            {headLedFrontLeft1Actuator,
                                    headLedFrontLeft0Actuator,
                                    headLedMiddleLeft0Actuator,
                                    headLedRearLeft0Actuator,
                                    headLedRearLeft1Actuator,
                                    headLedRearLeft2Actuator,
                                    headLedRearRight2Actuator,
                                    headLedRearRight1Actuator,
                                    headLedRearRight0Actuator,
                                    headLedMiddleRight0Actuator,
                                    headLedFrontRight0Actuator,
                                    headLedFrontRight1Actuator},
                            color);
                    break;

                // avoid compiler warning (these groups were rewritten in handleRequest())
                case LED::RIGHT_EYE:
                case LED::LEFT_EYE:
                case LED::ALL:
                    break;
            }

            // don't forget to update internal data structure
            _internalState[group] = color;
        }

        _requestedLeds.clear();
        return RUNNING;
    }

    /**
     * Handle commands
     */
    void handleRequest(SetLeds req) {
        // skip if request color is already set
        if (_internalState[req.group] == req.color)
            return;

        // handle LED groups, that consist of multiple sub-groups
        switch (req.group) {
            case LED::BRAIN:
                _requestedLeds[LED::BRAIN_LEFT] = req.color;
                _requestedLeds[LED::BRAIN_RIGHT] = req.color;
                break;
            case LED::RIGHT_EYE:
                _requestedLeds[LED::RIGHT_EYE_LEFT] = req.color;
                _requestedLeds[LED::RIGHT_EYE_RIGHT] = req.color;
                break;
            case LED::LEFT_EYE:
                _requestedLeds[LED::LEFT_EYE_LEFT] = req.color;
                _requestedLeds[LED::LEFT_EYE_RIGHT] = req.color;
                break;
            case LED::ALL:
                _requestedLeds[LED::CHEST] = req.color;
                _requestedLeds[LED::BRAIN_LEFT] = req.color;
                _requestedLeds[LED::BRAIN_RIGHT] = req.color;
                _requestedLeds[LED::RIGHT_EYE_LEFT] = req.color;
                _requestedLeds[LED::RIGHT_EYE_RIGHT] = req.color;
                _requestedLeds[LED::LEFT_EYE_LEFT] = req.color;
                _requestedLeds[LED::LEFT_EYE_RIGHT] = req.color;
                _requestedLeds[LED::RIGHT_EAR] = req.color;
                _requestedLeds[LED::LEFT_EAR] = req.color;
                _requestedLeds[LED::RIGHT_FOOT] = req.color;
                _requestedLeds[LED::LEFT_FOOT] = req.color;
                break;
            default:
                _requestedLeds[req.group] = req.color;
        }
    }

private:
    std::unordered_map<LED, int> _internalState, ///< store internal representation to avoid double set of values
            _requestedLeds;                      ///< leds requested. will be set with updateActuators call

    void updateEar(float *actuators, int actuator, size_t value) {
        static const size_t num_actuators = 10;

        value = std::min(num_actuators, (value * num_actuators) / 100);

        actuatorUpdateRange(actuators, actuator, actuator + num_actuators, 0);
        for (size_t i = 0; i < value && i < num_actuators; i++) {
            actuators[actuator + i] = 0xFF;
        }
    }

    void updateBrain(float *actuators, std::vector<int> segments, size_t value) {
        size_t num_actuators = segments.size();

        value = std::min(num_actuators, (value * num_actuators) / 100);

        for (const int &segment : segments) {
            actuators[segment] = 0;
        }

        for (size_t i = 0; i < value && i < segments.size(); i++) {
            actuators[segments[i]] = 0xFF;
        }
    }

    inline void actuatorUpdateRange(float *actuators, int from, int to, int value) {
        for (int i = from; i <= to; ++i)
            actuators[i] = value;
    }
};
