#pragma once

#include <framework/blackboard/blackboard.h>
#include <libbembelbots/config/botnames.h>
#include <representations/bembelbots/types.h>

#include <string>
#include <string_view>

struct ConfigFiles;

class SettingsBlackboard : public Blackboard {

public:
    explicit SettingsBlackboard();
    ~SettingsBlackboard();
    bool loadConfig(ConfigFiles &) override;
    bool writeConfig(ConfigFiles &) override;
    void logSettings();

    std::string nameToStr(const RobotName &n) const;
    std::string roleToStr(const RobotRole &r) const;

    void check_simulation();

    MAKE_VAR(std::string, logPath);
    std::string configPath;
    std::string motionsPath;

    MAKE_VAR(bool, simulator);
    MAKE_VAR(bool, docker);
    MAKE_VAR(bool, isPenaltyShootout);
    MAKE_VAR(bool, logToFile);
    MAKE_VAR(bool, logImages);
    MAKE_VAR(int,  logImagesInterval);
    MAKE_VAR(FieldSize, fieldSize);
    MAKE_VAR(int, id);
    MAKE_VAR(int, teamNumber);
    MAKE_VAR(RobotName, name);
    MAKE_VAR(RobotRole, role);
    std::string simulatorHost;
    int instance;
};

std::ostream &operator<<(std::ostream &s, const SettingsBlackboard *rhs);

// vim: set ts=4 sw=4 sts=4 expandtab:
