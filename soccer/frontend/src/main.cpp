#include <framework/rt/kernel.h>
#include <chrono>
#include <thread>
#include "system.h"
#include "config.h"
#include "network/debugserver.h"
#include "logfile/logfile.h"

#include <representations/blackboards/settings.h>
#include <representations/bembelbots/nao_info.h>
#include <framework/util/clock_simulator.h>
#include <modules/robocup.h>

struct Application : public Engine {
    rt::Context<SettingsBlackboard, rt::Write> settings;
    rt::Context<NaoInfo, rt::Write> nao;
    rt::Context<PlayingField, rt::Write> playingfield;
    rt::Context<Config, rt::Write> config;

    bool setup(int use_instance, bool docker) {
        settings->instance = use_instance;
        settings->docker = docker;
        settings->check_simulation();

        NaoInfoReader naoInfoReader;
        NaoInfoResult naoInfoResult = naoInfoReader.getNaoInfo(settings->instance, settings->docker);

        if(naoInfoResult.error == NaoInfoReaderError::BACKEND_CONNECT_ERROR) {
            LOG_ERROR << "unable to connect to backend";
            return false;
        }

        if(naoInfoResult.error == NaoInfoReaderError::BACKEND_READ_ERROR) {
            LOG_ERROR << "unable to read nao info from shared memory";
            return false;
        }

        *nao = naoInfoResult.naoInfo;

        std::string baseDir("/home/nao");

        if (settings->simulator) {
            char *test = getenv("AL_DIR");
            if (!test) {
                LOG_ERROR << "AL_DIR not set";
                return false;
            }
            baseDir = test;
            useSimulatorClock(); // set clock to fake time
        }

        LOG_INFO << "Hi, I am " << nao->getName();
        settings->configPath = baseDir + "/config/";
        LOG_INFO << "determine config path " << settings->configPath;
        settings->motionsPath = baseDir + "/motions/";
        LOG_INFO << "determine motions path " << settings->motionsPath;

        if (settings->configPath.empty()) {
            LOG_ERROR << "No path to config file!";
            return false;
        }

        return true;
    }

    int run(int use_instance, bool docker, System *system) {
        rt::Kernel soccer;

        DebugServer network;
        RoboCup robocup;
        LogFile logfile;

        soccer.hook("App", [&](rt::Linker &link) {
            link(settings);
            link(nao);
            link(playingfield);
            link(config);
        });
        
        soccer.loadLogger(&logfile);

        soccer.load(&robocup);
        soccer.load(&network);

        auto [modulesOk, modulesErrorMsg] = soccer.compile();
        if (not modulesOk) {
            LOG_ERROR << modulesErrorMsg;
            return EXIT_FAILURE;
        }

        LOG_DEBUG << std::endl << soccer.printModules();

        if(not setup(use_instance, docker)) {
            return EXIT_FAILURE;
        }
       
        config->loadBaseSettings(nao, settings, playingfield);
        soccer.setup();
        config->loadBlackboardSettings(settings);
        soccer.start();

        const std::chrono::milliseconds waitTime{30};
        while (System::is_running) {
            std::this_thread::sleep_for(waitTime);
        }
        LOG_WARN << "Main loop finished, stopping frontend ...";

        soccer.stop();
        system->stop();

        return EXIT_SUCCESS;
    }
};

int main(int argc, char *argv[])
{
    System system;
    if (!system.init(argc, argv)) {
        return EXIT_FAILURE;
    }

    system.set_engine<Application>();

    return system.run();
}
