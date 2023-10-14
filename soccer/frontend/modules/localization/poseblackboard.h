#pragma once

#include <framework/blackboard/blackboard.h>
#include <framework/math/directed_coord.h>
#include <framework/util/clock.h>

class PoseBlackboard : public Blackboard {
public:
    PoseBlackboard();
    ~PoseBlackboard();

    MAKE_VAR(std::vector<DirectedCoord>, Hypotheses);
    MAKE_VAR(std::vector<DirectedCoord>, Particles);

    // groundtruth position & timestamp
    MAKE_VAR(DirectedCoord, gtPosition);
    MAKE_VAR(TimestampMs, gtTimestamp);
};

// vim: set ts=4 sw=4 sts=4 expandtab:
