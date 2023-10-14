#pragma once
#include <framework/rt/module.h>
#include "naostate.h"

struct BBActuatorData;
class  BBSensorData;
struct NaoModules;

using SensorCallback = std::function<void(NaoState &)>;

class Nao : public rt::NoThreadModule {
public:
    Nao();
    void load(rt::Kernel &) override;
    void connect(rt::Linker &) override;
    void stop() override;
    void set_sensor_callback(SensorCallback);
private:
    std::shared_ptr<NaoModules> modules {nullptr};
};
