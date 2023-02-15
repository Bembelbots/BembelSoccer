#include "settings.h"

#include <boost/asio/ip/host_name.hpp>
#include <framework/util/configparser.h>
#include <framework/util/filesystem.h>
#include <framework/util/getenv.h>
#include <framework/common/platform.h>
#include <framework/logger/logger.h>

#include "../camera/camera.h"
#include <fstream>
#include <string_view>

SettingsBlackboard::SettingsBlackboard() : Blackboard("RobotSettings") {
    INIT_VAR(id, 0, "id number of robot (real bot: config file, simulator: got from port number)");
    INIT_VAR(teamNumber, 0, "team id, see RoboCupGameControlData");

    INIT_VAR(logPath, getTmpDir(), "path to store log files");

    INIT_VAR(simulator, false, "is this a simulated robot?");
    INIT_VAR(docker, false, "running simulator inside docker container?");

    INIT_VAR_RW(isPenaltyShootout,
            false,
            "true if bot is either configured as penalty kicker/goalie or enabled by gamecontroller");

    INIT_VAR_RW(storeRawImages, false, "flag, which decides to save raw images @note slows down framework!");
    INIT_VAR_RW(storeImgInterval,
            camera::fps,
            "write every Nth image (1: store all images, 2: skip every other image, etc..)");
    INIT_VAR(logToFile, false, "enable logging to file");

    INIT_ENUM(fieldSize, FieldSize::SPL, "size of playingfield");
    INIT_VAR(name, RobotName::UNKNOWN, "robot name");
    INIT_ENUM_RW(role, RobotRole::NONE, "robot role");
}

SettingsBlackboard::~SettingsBlackboard() {
}

std::string_view SettingsBlackboard::roleToStr(const RobotRole &r) const {
    return enumToStr(r);
}

std::string_view SettingsBlackboard::nameToStr(const RobotName &n) const {
    // workaround for playing on unknown robots: use hostname as identifier (same as jrlmonitor)
    if (n == name) {
        if (n == RobotName::UNKNOWN)
            return boost::asio::ip::host_name();
        else if (docker)
            return getEnvVar("ROBOT_NAME");
    }

    return DEFS::enum2botname(n);
}

void SettingsBlackboard::check_simulation() {
    // ensure simulator flag is set corrently early on startup
    simulator = docker || !fs::is_directory("/var/persistent/home/nao");
}

bool SettingsBlackboard::loadConfig(ConfigFiles &fcfg) {
    auto *cfg = fcfg.settings;

    READ_KEY(cfg, logToFile, bool);
    READ_KEY(cfg, storeRawImages, bool);
    READ_KEY_TRY(cfg, storeImgInterval, int);
    READ_KEY_TRY(cfg, logPath, std::string);

    READ_KEY_ENUM(cfg, fieldSize, FieldSize);

    READ_KEY(cfg, id, int);
    --id; // config file stores jersey number now, substract 1 so we can use id as array index

    READ_KEY(cfg, teamNumber, int);

    READ_KEY_ENUM(cfg, role, RobotRole);

    if (storeRawImages && !logToFile) {
        LOG_INFO << "storeRawImages is activated, forcing logToFile";
        logToFile = true;
    }

    LOG_INFO << "SETTINGS";
    LOG_INFO << this;

    return true;
}

bool SettingsBlackboard::writeConfig(ConfigFiles &fcfg) {
    auto *cfg = fcfg.settings;

    WRITE_KEY(cfg, logToFile, bool);
    WRITE_KEY(cfg, logPath, std::string);
    WRITE_KEY(cfg, storeRawImages, bool);
    WRITE_KEY(cfg, storeImgInterval, int);
    WRITE_KEY_ENUM(cfg, fieldSize);
    WRITE_KEY_VALUE(cfg, id, id + 1, int);
    WRITE_KEY(cfg, teamNumber, int);
    WRITE_KEY_ENUM(cfg, role);
    return true;
}

void SettingsBlackboard::logSettings() {
    LOG_DATA << "SettingsBlackboard: "
            << "name=" << nameToStr(name) << ";"
            << "id=" << id << ";"
            << "fieldSize=" << fieldSize << ";"
            << "teamNumber=" << teamNumber << ";"
            << "role=" << role << ";"
            << "logToFile=" << logToFile << ";"
            << "storeRawImages=" << storeRawImages << ";"
            << "storeImgInterval=" << storeImgInterval << ";"
            << "simulator=" << simulator;
}

std::ostream &operator<<(std::ostream &s, const SettingsBlackboard *rhs) {
    s << "current framework blackboard content:\n";
    s << "  logToFile:           " << rhs->logToFile << "\n";
    s << "  storeRawImages:      " << rhs->storeRawImages << "\n";
    s << "  storeImgInterval:    " << rhs->storeImgInterval << "\n";
    s << "  imagePath:           " << rhs->imagePath << "\n";
    s << "  fieldSize:           " << int(rhs->fieldSize) << "\n";
    s << "  id:                  " << rhs->id << "\n";
    s << "  teamNumber:          " << rhs->teamNumber << "\n";
    s << "  role:                " << int(rhs->role) << "\n";
    return s;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
