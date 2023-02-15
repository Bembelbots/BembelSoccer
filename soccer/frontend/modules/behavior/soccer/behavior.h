#pragma once

#include "definitions.h"

#include "behaviorblackboard.h"

#include <framework/logger/logger.h>
#include <representations/bembelbots/constants.h>
#include <framework/math/utils.h>

#include <algorithm>

// >>> CABSL should be included last <<<
#include <Cabsl.h>

class Blackboard;

// place Behavior in own namespace in order to allow:
// - CONST:: to be C::
// (without polluting the entire namespace!)
namespace BEHAVE_PRIVATE {

namespace C = CONSTANTS;

// Behavior wraps all options together
class Behavior : public Cabsl<Behavior>, public BehaviorBlackboard {

// all options are included here
// FIXME Behavior excluded!
#include "options.h"
#include "undef_cabsl.h"

public:
    microTime time_ms = 0;
    ActivationGraph activationGraph; /**< The activation graph used for debugging. */

    Behavior();

    // Delete copy / move semantic as these operations break the internal cabsl
    // state
    Behavior(const Behavior &) = delete;
    Behavior(Behavior &&) = delete;
    Behavior &operator=(Behavior &&) = delete;
    Behavior &operator=(const Behavior &) = delete;

    void execute(const std::vector<OptionInfos::Option> &roots);
private:
    std::string createGraphRepresentation();
    std::string createActivationGraphString();

};

} // namespace BEHAVE_PRIVATE

// vim: set ts=4 sw=4 sts=4 expandtab:
