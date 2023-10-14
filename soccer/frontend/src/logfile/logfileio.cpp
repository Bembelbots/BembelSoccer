#include "logfileio.h"

#include <filesystem>

#include <representations/vision/image.h>
#include <framework/logger/logger.h>

#include <framework/image/rgb.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

static constexpr uint8_t PNG_COMPRESSION_LVL{2};
static constexpr uint8_t JPEG_QUALITY{80};

namespace fs = std::filesystem;
using libzippp::ZipArchive;


LogFileIO::~LogFileIO() {
    zip->close();
    ZipArchive::free(zip.get());
}

void LogFileIO::connect(rt::Linker &link) {
    link.name = "LogFileIO";
    link(settings);
    link(logdata);
}

void LogFileIO::setup() {
    std::stringstream zipname;
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    zipname << settings->nameToStr(settings->name);
    zipname << "_" << std::put_time(std::localtime(&in_time_t), "%F-%T") << ".zip";

    fs::path zippath = settings->logPath;
    zippath /= zipname.str();

    zip = std::make_unique<ZipArchive>(zippath);
    if(not zip->open(ZipArchive::Write)) {
        LOG_WARN << "logfileio: unable to create ZIP file";
        zip = nullptr;
        return;
    }
    // disable compression
    zip->setCompressionMethod(libzippp::STORE);
}

void LogFileIO::process() {
    using namespace std::placeholders;

    logdata.waitWhileEmpty();

    if (!zip)
        return;

    zip->open(ZipArchive::Write);
    auto data = logdata.fetch();
    for(auto &context : data) {
        auto &context_data = context.data;
        context_data.handle<VisionImageProcessed>(std::bind(&LogFileIO::on_log_image, this, _1, _2), context.path);

        if(not zip->isOpen() || context_data.is_handled() || not context_data->is_serializeable()) {
            continue;
        }

        auto [size, content] = context_data->serialize();
        zip->addData(context.path, content, size);
    }
    if (zip->isOpen())
        zip->close();
}

void LogFileIO::on_log_image(std::filesystem::path path, rt::LogDataContainer<VisionImageProcessed>& image) {
    auto rgbImage = image->image.toRGB();
    auto imageFileData = encodeImage(rgbImage, ImageCodec::JPG);
    image->image.unlock(ImgLock::WRITER);

    if (zip->isOpen()) {
        // copy image buffer, so libzippp may free it, after data is written.
        // Use malloc() instead of new, since free() is called from libzip C-API
        void *imgbuf = malloc(imageFileData.size());
        memcpy(imgbuf, imageFileData.data(), imageFileData.size());
        zip->addData(path.c_str(), imgbuf, imageFileData.size(), true);
    }
}

std::vector<u_char> LogFileIO::encodeImage(const RgbImage &img, const ImageCodec &codec) {
    std::string ext;
    std::vector<int> parms;

    if (codec == ImageCodec::PNG) {
        ext = ".png";
        parms = {cv::IMWRITE_PNG_COMPRESSION, PNG_COMPRESSION_LVL};
    } else {
        ext = ".jpg";
        parms = {cv::IMWRITE_JPEG_QUALITY, JPEG_QUALITY};
    }

    std::vector<u_char> buf;
    cv::imencode(ext, img.mat(), buf, parms);

    return buf;
}
