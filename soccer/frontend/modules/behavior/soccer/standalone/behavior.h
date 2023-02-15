#pragma once

#include "debug.h"
#include "input.h"
#include "output.h"

#include <representations/bembelbots/types.h>

#include <memory>
#include <sstream>
#include <vector>

class ReactiveWalkBlackboard;
namespace BEHAVE_PRIVATE {
class Behavior;
} // namespace BEHAVE_PRIVATE

namespace stab {

class Behavior {

public:
    Behavior(RobotRole);
    Behavior(Behavior &&);
    ~Behavior();

    Behavior &operator=(Behavior &&);

    bool doDebugRequest(const BBSetRequest &);

    Output &execute(const Input &);

    Output &out() { return output; }
    const Output &out() const { return output; }

private:
    std::unique_ptr<ReactiveWalkBlackboard> rwcalib;
    std::unique_ptr<BEHAVE_PRIVATE::Behavior> behavior; // Use as pointer to avoid recompilation when behavior changes
    RobotRole role;
    std::stringstream log;

    Output output{};

    void update(const Input &);
    Output accumulateOutput();

    static bool nearestToBall(const Input &) ;

    std::vector<BlackboardEntry> dumpBlackboard() const;

    std::string flushLog();

};
    
} // namepsace stab

// vim: set ts=4 sw=4 sts=4 expandtab:
