#include "poseblackboard.h"

PoseBlackboard::PoseBlackboard() :
    Blackboard("PoseData") {

    // TODO: both! hypos and measurments are DirectedCoords, so use them!
    // cppcheck-suppress useInitializationList
    INIT_VAR(Hypotheses, std::vector<DirectedCoord>(),
             "hypotheses of the hypothesisGenerator implementation");

    INIT_VAR(Particles, std::vector<DirectedCoord>(),
             "hypotheses of the localization implementation");

    INIT_VAR(gtPosition, DirectedCoord(),
             "last received groundtruth position (head angle)");

    INIT_VAR(gtTimestamp, -1,
             "timestamp when last groundtruth message has been received");
}

PoseBlackboard::~PoseBlackboard() {
}


// vim: set ts=4 sw=4 sts=4 expandtab:
