#pragma once
#include <memory>
#include <framework/rt/module.h>
#include <framework/util/filesystem.h>
#include <representations/blackboards/settings.h>
#include <representations/bembelbots/nao_info.h>
#include <representations/playingfield/playingfield.h>

class ConfigParser;

class Config : public rt::ModuleLoader {
public:
    Config() = default;
    void connect(rt::Linker &) override;
    void load(rt::Kernel &) override;
    
    bool loadBaseSettings();
    bool loadBlackboardSettings();
    // save blackboard configurations
    bool saveSettings();
    bool saveSettings(const std::string &destination);

    ConfigFiles cfg;

private:
    rt::Context<SettingsBlackboard, rt::Write> settings;
    rt::Context<PlayingField, rt::Write> _playingfield;
    rt::Context<NaoInfo> nao;

    std::shared_ptr<ConfigParser> _settingsCfg;
    std::shared_ptr<ConfigParser> _calibrationCfg; 

    bool writeConfig();
};
