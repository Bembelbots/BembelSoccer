#pragma once

#include <framework/rt/module.h>
#include "framework/rt/endpoints/input.h"
#include "representations/motion/body_state.h"
#include "gamecontrol_generated.h"
#include "log_event_generated.h"


class LogEvents : public rt::Module {
public:
    void setup() override;
    void connect(rt::Linker &) override;
    void process() override;

private:
    using BodyQNS = std::bitset<NUM_OF_BODY_QUESTIONS>;

    rt::Input<BodyState> body;
    rt::Input<bbapi::GamecontrolMessageT, rt::Listen> gamecontrol;
    rt::Output<bbapi::LogEventT, rt::Event> event;

    bbapi::GamecontrolMessageT gc_state;
    BodyQNS body_qns;
};
