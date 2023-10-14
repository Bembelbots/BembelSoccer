#include "simplebehavior.h"
#include <framework/logger/logger.h>
#include <framework/util/frame.h>
#include <framework/thread/util.h>

void SimpleBehavior::setup() {

}

void SimpleBehavior::connect(rt::Linker &link) {
    link.name = "SimpleBehavior";
    link(whistle);
    link(motion);
    link(settings);
    link(whistle_result);
}

void SimpleBehavior::process() {
    FrameScope frame(30ms);
    if(!once) {
        once = true;
        whistle.enqueue<WhistleStart>();
        //motion.enqueue<DoMotion>(Motion::STAND);
        motion.enqueue<NormalStand>();
        //sleep_for(500ms);
        //motion.enqueue<WalkMotion>(DirectedCoord(1.0f, 0.0f, Angle(0_deg)));
        //sleep_for(10s);
        //motion.enqueue<NormalStand>();
    }
    for(auto &result : whistle_result.fetch()) {
        LOG_DEBUG_IF(result.found) << "Whistle Found!";
    }
}

void SimpleBehavior::stop() {
    once = false;
}
