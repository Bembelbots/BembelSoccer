#include "settings.h"

#include <fstream>
#include <filesystem>
#include <system_error>
#include <string_view>

#include <boost/asio/ip/host_name.hpp>
#include <framework/util/configparser.h>
#include <framework/util/getenv.h>
#include <framework/common/platform.h>
#include <framework/logger/logger.h>

#include "../camera/camera.h"
#include <representations/bembelbots/constants.h>
#include <roles_generated.h>

SettingsBlackboard::SettingsBlackboard() : Blackboard("RobotSettings") {
    INIT_VAR(id, 0, "id number of robot (jerseyNumber in config file)");
    INIT_VAR(teamNumber, 0, "team id, see RoboCupGameControlData");

    INIT_VAR(logPath, getTmpDir(), "path to store log files");

    INIT_VAR(simulator, false, "is this a simulated robot?");
    INIT_VAR(docker, false, "running simulator inside docker container?");

    INIT_VAR_RW(logImages, false, "flag, which decides to save images");
    INIT_VAR_RW(logImagesInterval,
            camera::fps,
            "write every Nth image (1: store all images, 2: skip every other image, etc..)");
    INIT_VAR(logToFile, false, "enable logging to file");

    INIT_ENUM(fieldSize, FieldSize::SPL, "size of playingfield");
    INIT_VAR(name, RobotName::UNKNOWN, "robot name");
    INIT_ENUM_RW(role, RobotRole::NONE, "robot role");
}

SettingsBlackboard::~SettingsBlackboard() {
}

std::string SettingsBlackboard::roleToStr(const RobotRole &r) const {
    return bbapi::EnumNameRobotRole(r);
}

std::string SettingsBlackboard::nameToStr(const RobotName &n) const {
    // workaround for playing on unknown robots: use hostname as identifier (same as jrlmonitor)
    if (n == name) {
        if (docker)
            return getEnvVar("ROBOT_NAME");
        else if (n == RobotName::UNKNOWN)
            return boost::asio::ip::host_name();
    }

    return DEFS::enum2botname(n);
}

void SettingsBlackboard::check_simulation() {
    // ensure simulator flag is set corrently early on startup
    std::error_code ec;
    simulator = docker || !std::filesystem::is_regular_file("/sys/qi/robot_type", ec);
}

bool SettingsBlackboard::loadConfig(ConfigFiles &fcfg) {
    auto *cfg = fcfg.settings;

    READ_KEY(cfg, logToFile, bool);
    READ_KEY_TRY(cfg, logImages, bool);
    READ_KEY_TRY(cfg, logImagesInterval, int);
    READ_KEY_TRY(cfg, logPath, std::string);

    READ_KEY_ENUM(cfg, fieldSize, FieldSize);

    int jerseyNumber{0};
    READ_KEY(cfg, jerseyNumber, int);
    jsassert(jerseyNumber > 0 && jerseyNumber <= NUM_PLAYERS) << "Invalid jersey number in confg file: " << jerseyNumber;
    id = jerseyNumber - 1; // config file stores jersey number, substract 1 so we can use id as array index

    READ_KEY(cfg, teamNumber, int);

    READ_KEY_ENUM(cfg, role, RobotRole);

    if (logImages && !logToFile) {
        LOG_INFO << "logImages is activated, forcing logToFile";
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
    WRITE_KEY(cfg, logImages, bool);
    WRITE_KEY(cfg, logImagesInterval, int);
    WRITE_KEY_ENUM(cfg, fieldSize);
    WRITE_KEY_VALUE(cfg, jerseyNumber, id + 1, int);
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
            << "role=" << roleToStr(role) << ";"
            << "logToFile=" << logToFile << ";"
            << "logImages=" << logImages << ";"
            << "logImagesInterval=" << logImagesInterval << ";"
            << "simulator=" << simulator;
}

std::ostream &operator<<(std::ostream &s, const SettingsBlackboard *rhs) {
    s << "current framework blackboard content:\n";
    s << "  logToFile:           " << rhs->logToFile << "\n";
    s << "  logImages:           " << rhs->logImages << "\n";
    s << "  logImagesInterval:   " << rhs->logImagesInterval << "\n";
    s << "  fieldSize:           " << int(rhs->fieldSize) << "\n";
    s << "  id:                  " << rhs->id << "\n";
    s << "  teamNumber:          " << rhs->teamNumber << "\n";
    s << "  role:                " << int(rhs->role) << "\n";
    return s;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
