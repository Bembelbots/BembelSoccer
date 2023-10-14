#include "logfile.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>

#include <framework/thread/util.h>
#include <framework/util/frame.h>
#include <framework/rt/kernel.h>
#include <representations/motion/body_state.h>
#include <representations/flatbuffers/flatbuffers.h>
#include <flatbuffers/minireflect.h>

#include <representations/vision/image.h>
#include <representations/bembelbots/constants.h>

namespace fs = std::filesystem;

/*
 * <tick>/<output_name>.bin
 * <tick>/topcam.<ext>
 * <tick>/botcam.<ext>
 **/

void LogFile::load(rt::Kernel &soccer) {
    soccer.load(&io);
}

void LogFile::connect(rt::Linker &link) {
    link.name = "LogFile";
    link(loggers);
    link(settings);
    link(gamecontrol);
    link(out);
}

void LogFile::setup() {
    if(not settings->logToFile) {
        setup_success = false;
        LOG_INFO << "logfile: logToFile disabled";
        return;
    }
    LOG_INFO << "logfile: logToFile enabled";

    if(not fs::is_directory(settings->logPath, filesystem_error)) {
        setup_success &= fs::create_directories(settings->logPath, filesystem_error);
    }

    if(not setup_success) {
        LOG_WARN << "logfile: unable to create directories";
    }
}

void LogFile::process() {
    loggers->wait(12ms);
    
    if(not loggers->hasData()) {
        return;
    }

    log_images = settings->logImages;
    log_images &= (*gamecontrol)->gameState == GameState::PLAYING || (*gamecontrol)->gameState == GameState::READY || (*gamecontrol)->gameState == GameState::SET;
    log_images &= (*gamecontrol)->penalty == Penalty::NONE;
    log_images &= !(*gamecontrol)->unstiff;

    std::stringstream tickFolderName;
    tickFolderName << ticksPath << "/" << std::setw(10) << std::setfill('0') << tick;

    tickDir = tickFolderName.str();

    for(auto logger : *loggers) {
        auto module = loggers->getModule(logger);
        logger->consume_all([=](rt::LogData data) {
            this->on_logdata(module, data);
        });
    }
}

void LogFile::on_logdata(const rt::ModuleMeta &module, rt::LogData &data) {
    using namespace std::placeholders;

    data.handle<VisionImageProcessed>(std::bind(&LogFile::on_log_image, this, _1, _2, _3), module, data);
    data.handle<BodyState>([=](rt::LogDataContainer<BodyState>& state){
        tick++;
    });

    if(data.is_handled() || not data->is_serializeable()) {
       return;
    }

    if(not setup_success) {
        return;
    }

    auto out_file = tickDir / (data->name() + ".bin");
    out.emit(LogFileContext(out_file, data));
}

void LogFile::on_log_image(const rt::ModuleMeta &module, rt::LogData &data, rt::LogDataContainer<VisionImageProcessed>& image) {
    if(not setup_success || not log_images) {
        image->image.unlock(ImgLock::WRITER);
        return;
    }

    std::stringstream imageFileName;
    imageFileName << settings->nameToStr(settings->name) << "-" << std::setw(8) << std::setfill('0') << tick;
    if (image->image.camera == TOP_CAMERA) {
        imageFileName << "-top";
    }
    else {
        imageFileName << "-bottom";
    }

    imageFileName << ".jpg";

    auto imagePath = tickDir / imageFileName.str();
    out.emit(LogFileContext(imagePath, data));
}
