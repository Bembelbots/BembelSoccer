#include "config.h"

#include <exception>
#include <filesystem>
#include <system_error>

#include <framework/blackboard/blackboard.h>
#include <framework/blackboard/blackboardregistry.h>
#include <framework/logger/logger.h>
#include <framework/util/configparser.h>
#include <framework/thread/util.h>
#include <framework/util/clock.h>

namespace fs = std::filesystem;
static const fs::path USB_DRIVE{"/mnt/usb"};
static const fs::path FIELD_DIMENSIONS_JSON{"field_dimensions.json"};

void setupCfg(const NaoInfo &nao, std::shared_ptr<ConfigParser> &cfg, std::string index,
                                   const std::string &cfgFileName) {
    if (cfg) {
        return;
    }

    LOG_DEBUG << "Use config file: " << cfgFileName;
    cfg = std::make_shared<ConfigParser>(cfgFileName, index);
}

bool Config::loadBaseSettings(NaoInfo &nao, SettingsBlackboard *settings, PlayingField *_playingfield) {
    const fs::path cfg_path = settings->configPath;
    static const fs::path bb_json{"bembelbots.json"};
    static const fs::path cal_json{"calibration.json"};
    std::error_code ec;
    std::string settings_index = nao.getHostname();
    std::string calibration_index = settings_index;
        
    settings->name = nao.getNameId();

    if(settings->simulator) {
        calibration_index = "simulator";
    }

    bool usb_conf{false}, usb_cal{false};

    if (fs::is_regular_file(USB_DRIVE/bb_json, ec)) {
        setupCfg(nao, _settingsCfg, settings_index, USB_DRIVE / bb_json);
    } else { 
        setupCfg(nao, _settingsCfg, settings_index, cfg_path / bb_json);
    }

    if (fs::is_regular_file(USB_DRIVE/cal_json, ec)) {
        setupCfg(nao, _calibrationCfg, calibration_index, USB_DRIVE / cal_json);
    } else {
        setupCfg(nao, _calibrationCfg, calibration_index, cfg_path / cal_json);
    }
    
    cfg = {_settingsCfg.get(), _calibrationCfg.get()};

    LOG_DEBUG << "Load config for blackboard: " << settings->getBlackboardName();
    settings->loadConfig(cfg);

    if (fs::is_regular_file(USB_DRIVE / FIELD_DIMENSIONS_JSON, ec)) {
        settings->fieldSize = FieldSize::JSON;
        *_playingfield = PlayingField(USB_DRIVE / FIELD_DIMENSIONS_JSON);
    } else {
        *_playingfield = PlayingField(settings->fieldSize);
    }
    LOG_INFO << "create Playingfield (" << settings->fieldSize << ")";

    fs::path logPath = settings->logPath;
    fs::path c = fs::weakly_canonical(logPath, ec);
    if (!ec && !c.empty())
        logPath = c;
    
    settings->logPath = logPath.string();

    LOG_FLUSH();
    return true;
}

bool Config::loadBlackboardSettings(SettingsBlackboard &settings) {
    for (auto *bb : BlackboardRegistry::GetBlackboards()) {
        // settings blackboard should have loaded config by now
        if(bb->getBlackboardName() == settings.getBlackboardName()) {
            continue;
        }

        LOG_DEBUG << "Load config for blackboard: " << bb->getBlackboardName();
        if (!bb->loadConfig(cfg)) {
            LOG_ERROR << "failed to load config: " << bb->getBlackboardName();
            return false;
        }
    }

    LOG_FLUSH();
    return true;
}

bool Config::saveSettings() {
    if (!_settingsCfg || !_calibrationCfg) {
        LOG_ERROR << "Config parser are not setup.";
        LOG_ERROR << "Unable to save settings.";
        return false;
    }

    writeConfig();

    _settingsCfg->save();
    _calibrationCfg->save();

    return true;
}

bool Config::saveSettings(const std::string &destination) {
    if (!_settingsCfg || !_calibrationCfg) {
        LOG_ERROR << "Config parser are not setup.";
        LOG_ERROR << "Unable to save settings.";
        return false;
    }


    bool ok = writeConfig();
    if (!ok) {
        return false;
    }

    _settingsCfg->save(destination + std::string("/bembelbots.json"));
    _calibrationCfg->save(destination + std::string("/calibration.json"));

    return true;
}

bool Config::writeConfig() {
    ConfigFiles cfg{_settingsCfg.get(), _calibrationCfg.get()};

    bool ok = true;
    for (auto *bb : BlackboardRegistry::GetBlackboards()) {
        ok &= bb->writeConfig(cfg);
    }

    return ok;
}
