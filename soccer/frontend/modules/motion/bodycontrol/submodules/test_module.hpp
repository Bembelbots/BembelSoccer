#include <iostream>
#include <bodycontrol/environment/logger.h>

#ifndef TEST
    #define TEST if (false) std::ostream(0)
#endif

class TestModule : public SubModule {
public:
    TestModule(string name_):name(name_){
        TEST << "initialized " << name;
    };

private:
    string name;
public:
    SubModuleReturnValue step(BodyBlackboard * bb) override {
        TEST << "step " << name;
        return SubModuleReturnValue::MOTION_STABLE;
    }

};




