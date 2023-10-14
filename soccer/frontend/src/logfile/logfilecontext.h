#pragma once

#include <filesystem>
#include <framework/rt/logdata/logdata.h>

class LogFileContext {
public:
    std::filesystem::path path;
    rt::LogData data;

    LogFileContext(std::filesystem::path path, rt::LogData data)
        : path(path), data(data) {

    }
};