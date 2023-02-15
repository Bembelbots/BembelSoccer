#pragma once

#include "bodycontrol/internals/submodule.h"

class DoNothing : public SubModule {
public:
    SubModuleReturnValue step(BodyBlackboard * bb) override {
        return DEACTIVATE_ME;
    }
};
