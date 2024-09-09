#include "bodycontrol/internals/submodule.h"
#include "lola_names_generated.h"

#include <framework/logger/logger.h>

template<bbapi::JointNames id>
class SensorDumper : public SubModule {
public:
    SensorDumper(std::string_view name) : name(name) {}

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        LOG_DEBUG_EVERY_N(100) << " DUMP " << name << ": " << bb->sensors.joints.position[int(id)];
        return SubModuleReturnValue::RUNNING;
    }

private:
    std::string_view name;
};




