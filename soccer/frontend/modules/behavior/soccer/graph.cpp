#include "graph.h"
#include <map>
#include <string>

namespace BehaviorGraph {

std::string toJson() {
    std::ostringstream oss;

    pt::write_json(oss, toPropertyTree());

    return oss.str();
}

pt::ptree toPropertyTree() {
    #include <behavior_dot.h>

    pt::ptree root;

    for (auto graph : graphs) {
        pt::ptree list_item;

        list_item.put("", graph.second);

        root.push_back(std::make_pair(graph.first, list_item));
    }

    return root;
}

};
