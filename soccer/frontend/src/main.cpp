#include <framework/rt/kernel.h>
#include <chrono>
#include <thread>
#include "system.h"
#include "config.h"
#include "network/debugserver.h"

#include <representations/blackboards/settings.h>
#include <representations/bembelbots/nao_info.h>
#include <framework/util/clock_simulator.h>
#include <modules/robocup.h>

struct Application : public Engine {
    int run(int use_instance, bool docker, System *system) {
        rt::Kernel soccer;

        rt::Context<SettingsBlackboard, rt::Write> settings;
        rt::Context<NaoInfo, rt::Write> nao;
        rt::Context<Config, rt::Write> config;

        soccer.hook("App", [&](rt::Linker &link) {
            link(settings);
            link(nao);
            link(config);
        });
        soccer.load(config);
        
        settings->instance = use_instance;
        settings->docker = docker;
        settings->check_simulation();

        NaoInfoReader naoInfoReader;
        NaoInfoResult naoInfoResult = naoInfoReader.getNaoInfo(settings->instance, settings->docker);

        if(naoInfoResult.error == NaoInfoReaderError::BACKEND_CONNECT_ERROR) {
            LOG_ERROR << "unable to connect to backend";
            return EXIT_FAILURE;
        }

        if(naoInfoResult.error == NaoInfoReaderError::BACKEND_READ_ERROR) {
            LOG_ERROR << "unable to read nao info from shared memory";
            return EXIT_FAILURE;
        }

        *nao = naoInfoResult.naoInfo;

        std::string baseDir("/home/nao");

        if (settings->simulator) {
            char *test = getenv("AL_DIR");
            if (!test) {
                LOG_ERROR << "AL_DIR not set";
                exit(EXIT_FAILURE);
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
            return EXIT_FAILURE;
        }

        DebugServer network;
        RoboCup robocup;
        
        soccer.load(&robocup);
        soccer.load(&network);

        auto [modulesOk, modulesErrorMsg] = soccer.resolve();
        if (not modulesOk) {
            LOG_ERROR << modulesErrorMsg;
            return EXIT_FAILURE;
        }
        
        config->loadBaseSettings();
        soccer.setup();
        config->loadBlackboardSettings();
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
