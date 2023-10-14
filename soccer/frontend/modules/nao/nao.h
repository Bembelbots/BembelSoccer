#pragma once
#include <framework/rt/module.h>
#include "naostate.h"

struct BBActuatorData;
struct BBSensorData;
struct NaoModules;

using SensorCallback = std::function<void(NaoState &)>;

class Nao : public rt::ModuleLoader {
public:
    Nao();
    ~Nao();
    void load(rt::Kernel &) override;
    void stop() override;
    void set_sensor_callback(SensorCallback);
private:
    NaoModules* modules{nullptr};
};
