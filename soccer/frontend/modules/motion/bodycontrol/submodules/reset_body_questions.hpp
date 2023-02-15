#pragma once

#include <bodycontrol/internals/submodule.h>

class ResetBodyQuestions : public SubModule {
    
    SubModuleReturnValue step(BodyBlackboard *bb) override {
        bb->prevQns = bb->qns;
        bb->qns.reset();
        return RUNNING;
    }

};
