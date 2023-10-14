#include "depth_first_search.h"

using namespace rt;

std::vector<CircularDep> DepthFirstSearch::findCycles(gsl::span<ModuleMeta> modules) {
    moduleVisited.clear();
    moduleVisited.resize(modules.size());

    moduleFinished.clear();
    moduleFinished.resize(modules.size());

    std::vector<CircularDep> circularDeps;
    for (auto &mod : modules) {
        if (moduleFinished.at(mod.id)) {
            continue;
        }
        std::optional<CircularDep> maybeCycle = visit(mod.id, modules);
        if (maybeCycle.has_value()) {
            circularDeps.push_back(*maybeCycle);
        }
    }

    return circularDeps;
}

std::optional<CircularDep> DepthFirstSearch::visit(ModuleId id, gsl::span<ModuleMeta> modules) {
    moduleVisited.at(id) = true;

    std::optional<CircularDep> cycle;
    for (auto &child : modules[id].requiredBy) {
        if (moduleVisited.at(child)) {
            cycle = {child, id};
            break;
        }

        if (not moduleFinished.at(child)) {
            cycle = visit(child, modules);
            if (cycle.has_value()) {
                break;
            }
        }
    }

    moduleVisited.at(id) = false;
    moduleFinished.at(id) = true;

    return cycle;
}
