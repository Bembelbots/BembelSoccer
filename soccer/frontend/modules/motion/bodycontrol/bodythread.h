#include <bodycontrol/bodycontrol.h>
#include <core/rt/module.h>
#include <shared/backend/bembelbots_shm.h>

#include <memory>
#include <string>

class SettingsBlackboard;

// TODO this class can be merged with bodycontrol
class BodyThread : public rt::Module {
public:
    BodyThread();

    void run();

    void stop();

    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;

public:
    BodyControl bc;

private:
    static const std::string threadName;

    std::unique_ptr<BembelbotsShm> shm;
    BBSensorData sensorData;
    BBActuatorData actuatorData;

    rt::Context<SettingsBlackboard> settings;
    rt::Dispatch<BodyCommand, rt::Handle> cmds;
    rt::Output<BodyState> state;

    int timeout = 0;
    int missedFrames = 0;
    int ticks = 0;
};
