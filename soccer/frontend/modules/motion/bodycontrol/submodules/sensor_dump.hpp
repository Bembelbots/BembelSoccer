#include "bodycontrol/internals/submodule.h"

#include <framework/logger/logger.h>
#include <libbembelbots/bembelbots.h>

template<LBBSensorIds id>
class SensorDumper : public SubModule {
public:
    SensorDumper(std::string_view name) : name(name) {}

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        LOG_DEBUG_EVERY_N(100) << " DUMP " << name << ": " << bb->sensors[id];
        return SubModuleReturnValue::RUNNING;
    }

private:
    std::string_view name;
};




