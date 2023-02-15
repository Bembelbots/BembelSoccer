#include "standalone/config.h"
#include "behavior.h"

#include <string>
#include <vector>

using namespace std;
using BEHAVE_PRIVATE::Behavior;


Behavior::Behavior() :
    Cabsl<Behavior>(&activationGraph), BehaviorBlackboard() {}

void Behavior::execute(const std::vector<OptionInfos::Option> &roots) {
    beginFrame(time_ms);

    //LOG_INFO << "new behavior iteration: " << time_ms;

    current_options.clear();
    jsassert(current_options.empty());

    for (auto &b: roots) {
        Cabsl<Behavior>::execute(b);

        if (cabsl_states_debug) {
            cabsl_states = createActivationGraphString();
        }

        for (const ActivationGraph::Node &activeOption : activationGraph.graph) {
            current_options.push_back({activeOption.option, activeOption.state});
        }
    }

    //LOG_INFO << "behavior iteration end: " << time_ms;

    endFrame();
}

std::string Behavior::createActivationGraphString() {
    std::stringstream behavior_debug;

    for (const ActivationGraph::Node &activeOption : activationGraph.graph) {
        // if not first item
        if (not behavior_debug.rdbuf()->in_avail() == 0) {
            behavior_debug  << ":";
        }

        behavior_debug << activeOption.depth - 1 << "-"
                       << activeOption.option.c_str() << ">" << activeOption.state.c_str();
    }

    return behavior_debug.str();
}

std::string Behavior::createGraphRepresentation() {
    std::stringstream behavior_debug;

    int prev_depth = -1;
    for (const ActivationGraph::Node &activeOption : activationGraph.graph) {
        int depth = activeOption.depth - 1;

        if (prev_depth == -1) {
            // do nothing
        } else if (prev_depth == depth) {
            behavior_debug << "|";
        } else if (prev_depth < depth) {
            behavior_debug << "[";
        } else if (prev_depth > depth and depth > 0) {
            behavior_debug << "]|";
        } else if (prev_depth > depth) {
            behavior_debug << "]";
        }

        behavior_debug << activeOption.option.c_str() << ">" <<
                       activeOption.state.c_str();
        prev_depth = depth;
    }

    while (prev_depth > 0) {
        behavior_debug << "]";
        prev_depth--;
    }

    return behavior_debug.str();
}

// vim: set ts=4 sw=4 sts=4 expandtab:
