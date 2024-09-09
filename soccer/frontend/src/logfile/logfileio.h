#pragma once

#include "logfilecontext.h"

#include <memory>
#include <filesystem>

#include <zip.h>
#include <libzippp/libzippp.h>

#include <framework/rt/module.h>
#include <representations/bembelbots/types.h>
#include <representations/blackboards/settings.h>

struct RgbImage;
struct VisionImageProcessed;
struct RefereeGestureDebug;

class LogFileIO : public rt::Module {
public:
    LogFileIO() = default;
    ~LogFileIO();

    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;

private:
    libzippp::ZipArchive *zip{nullptr};

    rt::Context<SettingsBlackboard> settings;
    rt::Input<LogFileContext, rt::Snoop> logdata;

    std::filesystem::path tickDir;
    std::filesystem::path ticksPath;

    void on_log_image(std::filesystem::path path, rt::LogDataContainer<VisionImageProcessed> &);
    void on_log_refereegesture(std::filesystem::path path, rt::LogDataContainer<RefereeGestureDebug> &);

    void close();

    bool zipWrite(const std::string &fname, const void *data, size_t length, const bool freeData = false,
            const libzippp::CompressionMethod &cm = libzippp::DEFAULT);
};
