#pragma once
#include <memory>
#include <filesystem>

#include <framework/rt/module.h>
#include <representations/blackboards/settings.h>
#include <string_view>

#include "framework/rt/endpoints/input.h"
#include "gamecontrol_generated.h"
#include "logfilecontext.h"
#include "logfileio.h"

struct VisionImageProcessed;
struct RefereeGestureDebug;

class LogFile : public rt::Module {
public:
    LogFile() = default;

    void load(rt::Kernel &) override;
    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;

    bool disabled() const override { return false; }

private:
    static constexpr std::string_view ticksPath{"ticks"};
    LogFileIO io;

    rt::Context<rt::LogDataContext, rt::Write> loggers;
    rt::Context<SettingsBlackboard> settings;

    rt::Output<LogFileContext, rt::Event | rt::DisableLogging> out;
    rt::Input<bbapi::GamecontrolMessageT, rt::Listen> gamecontrol;

    bool log_images = true;
    bool log_enabled = true;

    size_t tick = 0;
    std::filesystem::path tickDir;

    std::error_code filesystem_error;

    void on_logdata(const rt::ModuleMeta &, rt::LogData &);
    void on_log_image(const rt::ModuleMeta &, rt::LogData &, rt::LogDataContainer<VisionImageProcessed> &);
    void on_log_refereegesture(
            const rt::ModuleMeta &module, rt::LogData &data, rt::LogDataContainer<RefereeGestureDebug> &image);
};
