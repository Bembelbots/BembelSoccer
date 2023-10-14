#pragma once
#include <memory>
#include <framework/rt/module.h>
#include <representations/blackboards/settings.h>
#include <representations/bembelbots/nao_info.h>
#include <representations/playingfield/playingfield.h>

class ConfigParser;

class Config {
public:
    Config() = default;
    
    bool loadBaseSettings(NaoInfo &nao, SettingsBlackboard *settings, PlayingField *_playingfield);
    bool loadBlackboardSettings(SettingsBlackboard &settings);
    // save blackboard configurations
    bool saveSettings();
    bool saveSettings(const std::string &destination);

private:
    std::shared_ptr<ConfigParser> _settingsCfg;
    std::shared_ptr<ConfigParser> _calibrationCfg;

    ConfigFiles cfg;

    bool writeConfig();
};
