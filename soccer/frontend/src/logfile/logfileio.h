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

class LogFileIO : public rt::Module {
public:
    LogFileIO() = default;
    ~LogFileIO();

    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;

private:
    std::unique_ptr<libzippp::ZipArchive> zip{nullptr};

    rt::Context<SettingsBlackboard> settings;
    rt::Input<LogFileContext, rt::Snoop> logdata;

    std::filesystem::path tickDir;
    std::filesystem::path ticksPath;

    void on_log_image(std::filesystem::path path, rt::LogDataContainer<VisionImageProcessed>&);

    std::vector<u_char> encodeImage(const RgbImage &img, const ImageCodec &codec);
};
