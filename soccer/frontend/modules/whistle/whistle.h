#pragma once
#include <framework/rt/module.h>
#include <representations/whistle/commands.h>
#include <representations/whistle/whistleresult.h>
#include <representations/blackboards/settings.h>

class WhistleDetector;
class AlsaRecorder;

class Whistle : public rt::Module {
public:
    void setup() override;
    void connect(rt::Linker &) override;
    void stop() override;
    void process() override;

private:
    std::shared_ptr<WhistleDetector> detector;
    std::shared_ptr<AlsaRecorder> recorder;

    rt::Context<SettingsBlackboard> settings;
    rt::Dispatch<WhistleCommand, rt::Handle> cmds;
    rt::Output<WhistleResult, rt::Event> event;

    void onRecordOnly(WhistleRecordOnly &);
    void onStart(WhistleStart &);
    void onStop(WhistleStop &);

    bool listening = false;
};
