#pragma once

#include <behavior/standalone/debug.h>

#include <core/util/mathtoolbox.h>
#include <bodycontrol/motion.h>

#include <ActivationGraph.h>


namespace stab {

struct Output {
    DirectedCoord walk;
    Motion bm_type;
    float headYawSpeed;
    ActivationGraph behaviorGraph;
    std::string log;
    std::vector<BlackboardEntry> blackboard;
    Coord rw_debug_target_dribble; 
    DirectedCoord rw_debug_target_approach_wcs;
};
    
} // namespace stab

// vim: set ts=4 sw=4 sts=4 expandtab:
