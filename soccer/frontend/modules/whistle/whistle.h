#pragma once
#include <framework/rt/module.h>
#include <representations/blackboards/settings.h>
#include <whistle_message_generated.h>
#include <whistle_commands_generated.h>

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
    rt::Command<bbapi::WhistleCommandT, rt::Handle> cmds;
    rt::Output<bbapi::WhistleMessageT, rt::Event> event;

    void onRecordOnly(bbapi::WhistleRecordOnlyT &);
    void onStart(bbapi::WhistleStartT &);
    void onStop(bbapi::WhistleStopT &);

    bool listening = false;
    bool prevFound = false;
};
