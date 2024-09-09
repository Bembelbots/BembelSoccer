#include "logfileio.h"

#include <exception>
#include <filesystem>

#include <libzippp.h>
#include <representations/vision/image.h>
#include <framework/logger/logger.h>

#include <framework/image/rgb.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <modules/refereegesture/refereegesture.h>

static constexpr uint32_t ZIP_COMPRESSION_LEVEL{3};
static constexpr libzippp::CompressionMethod ZIP_COMPRESSION{libzippp::DEFLATE};
static constexpr uint8_t PNG_COMPRESSION_LVL{2};
static constexpr uint8_t JPEG_QUALITY{80};

namespace fs = std::filesystem;
using libzippp::ZipArchive;

LogFileIO::~LogFileIO() {
    close();
    if (zip) {
        delete zip;
        zip = nullptr;
    }
}

void LogFileIO::connect(rt::Linker &link) {
    link.name = "LogFileIO";
    link(settings);
    link(logdata);
}

void LogFileIO::setup() {
    if (!settings->logToFile)
        return;

    fs::path zippath = settings->logPath;
    std::error_code filesystem_error;
    if (!fs::is_directory(zippath, filesystem_error)) {
        if (!fs::create_directories(zippath, filesystem_error)) {
            LOG_WARN << "logfileio: unable to create log directory " << zippath;
            return;
        }
    }

    std::stringstream zipname;
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    zipname << settings->nameToStr(settings->name);
    zipname << "_" << std::put_time(std::localtime(&in_time_t), "%F-%H%M%S") << ".zip";
    zippath /= zipname.str();

    zip = new ZipArchive(zippath);
    if (not zip->open(ZipArchive::Write)) {
        LOG_WARN << "logfileio: unable to create ZIP file " << zippath;
        delete zip;
        zip = nullptr;
        return;
    }

    // add config files to log
    zip->setCompressionMethod(ZIP_COMPRESSION);
    zip->setCompressionLevel(ZIP_COMPRESSION_LEVEL);
    for (const auto &f : {"bembelbots.json", "calibration.json"})
        zip->addFile(std::string("config/") + f, settings->configPath + f);
    close();
}

void LogFileIO::process() {
    using namespace std::placeholders;

    logdata.waitWhileEmpty();

    if (!zip)
        return;

    if (zip->open(ZipArchive::Write)) {
        auto data = logdata.fetch();
        for (auto &context : data) {
            auto &context_data = context.data;
            context_data.handle<VisionImageProcessed>(std::bind(&LogFileIO::on_log_image, this, _1, _2), context.path);
            context_data.handle<RefereeGestureDebug>(
                    std::bind(&LogFileIO::on_log_refereegesture, this, _1, _2), context.path);

            if (context_data.is_handled() || not context_data->is_serializeable()) {
                continue;
            }

            auto [size, content] = context_data->serialize();
            zipWrite(context.path, content, size, false, ZIP_COMPRESSION);
        }
        close();
    }
}

void LogFileIO::on_log_image(std::filesystem::path path, rt::LogDataContainer<VisionImageProcessed> &image) {
    if (zip) {
        // copy image buffer, so libzippp may free it, after data is written.
        // Use malloc() instead of new, since free() is called from libzip C-API
        auto &jpeg{*image->jpeg};
        void *imgbuf = malloc(jpeg.size());
        memcpy(imgbuf, jpeg.data(), jpeg.size());
        zipWrite(path.c_str(), imgbuf, jpeg.size(), true, libzippp::STORE); // store images uncompressed
    }
}

void LogFileIO::on_log_refereegesture(std::filesystem::path path, rt::LogDataContainer<RefereeGestureDebug> &image) {
    std::vector<int> parms{cv::IMWRITE_JPEG_QUALITY, 80};
    std::vector<u_char> jpeg;
    cv::imencode(path, image->img, jpeg, parms);

    void *imgbuf = malloc(jpeg.size());
    memcpy(imgbuf, jpeg.data(), jpeg.size());
    zipWrite(path.c_str(), imgbuf, jpeg.size(), true, libzippp::STORE); // store images uncompressed
}

void LogFileIO::close() {
    if (!zip)
        return;

    try {
        // file is not actually written, until ZipArchive::close() is called,
        // so this might throw, if file can't be written
        zip->close();
    } catch (std::exception &e) {
        LOG_ERROR << "logfileio: error writing to ZIP file " << zip->getPath();
        zip->discard();
        delete zip;
        zip = nullptr;
    }
}

bool LogFileIO::zipWrite(const std::string &fname, const void *data, size_t length, const bool freeData,
        const libzippp::CompressionMethod &cm) {
    if (!zip || !zip->open(ZipArchive::Write))
        return false;

    zip->setCompressionMethod(cm);
    zip->setCompressionLevel(ZIP_COMPRESSION_LEVEL);

    return zip->addData(fname, data, length, freeData);
}
