#pragma once

#include "meta.h"

#include <gsl/span>
#include <optional>
#include <utility>
#include <vector>

namespace rt {

using CircularDep = std::pair<ModuleId, ModuleId>;

class DepthFirstSearch {

public:
    std::vector<CircularDep> findCycles(gsl::span<ModuleMeta>);

private:
    std::vector<bool> moduleVisited;
    std::vector<bool> moduleFinished;

    std::optional<CircularDep> visit(ModuleId, gsl::span<ModuleMeta>);
};

} // namespace rt
