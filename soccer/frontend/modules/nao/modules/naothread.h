#pragma once
#include <functional>
#include <libbembelbots/bembelbots_shm.h>
#include <framework/rt/module.h>
#include <representations/bembelbots/nao_info.h>
#include "../naostate.h"
#include "../nao.h"

class SettingsBlackboard;

static constexpr int MAX_SENSOR_FETCH_MS = 13; //< Warn when getting sensors takes longer than this

class NaoThread : public rt::Module {
public:
    void set_sensor_callback(SensorCallback);

    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;
    void stop();

private:
    static const std::string threadName;

    SensorCallback sensor_callback;

    std::unique_ptr<BembelbotsShm> shm;

    rt::Context<NaoInfo> nao_info;
    rt::Context<SettingsBlackboard> settings;
    rt::Output<NaoState> nao_state;

    int timeout = 0;
    int missedFrames = 0;
    int ticks = 0;
};
