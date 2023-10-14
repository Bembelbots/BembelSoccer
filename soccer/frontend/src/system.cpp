#include "system.h"

#include <framework/logger/logger.h>
#include <framework/logger/backends.h>
#include <libbembelbots/bembelbots.h>
#include <iostream>

// linux-platform dependent
#include <sys/file.h>  // flock
#include <unistd.h>    // fork()
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>

#include <representations/blackboards/settings.h>

#include <framework/util/clock_simulator.h>
#include <framework/util/stacktrace.h>
#include <framework/util/buildinfo.hpp>

#include <glog/logging.h>

struct Instance
{
    Instance(int use_instance)
        : instance(use_instance) {
        std::stringstream tin;
        tin << "/tmp/bembelbots-" << instance;
        lock_fd = open(tin.str().c_str(), O_CREAT, 0600);
    }

    bool is_unique() {
        return !(lock_fd == -1 || flock(lock_fd, LOCK_EX | LOCK_NB) == -1);
    }

    ~Instance() {
        if (lock_fd != -1) {
            close(lock_fd);
        }
    }

private:
    int instance;
    int lock_fd = -1;
};

// global frontend status variable
bool System::is_running = true;

// global signal handler
static void sighandler_shutdown(int sig) {
    if (System::is_running) {
        std::cout << std::endl;
        LOG_ERROR << "Caught signal " << sig;
        LOG_ERROR << "Shutting down ...";
    }
    System::is_running = false;
}

static void terminate_handler() {
    LOG_ERROR << "Caught uncaught exception";
    printStackTrace();
    System::is_running = false;
}


static void segfault_handler(int sig, siginfo_t *si, void *) {
    LOG_ERROR << "Caught signal " << sig << " (" << strsignal(sig) << ")";
    printStackTrace();
}

static void register_handlers()
{
    /***************************************************************************/
    /** register handler for strg+c (2) and termination (15) signal            */

    signal(SIGTERM, sighandler_shutdown);
    signal(SIGINT, sighandler_shutdown);

    // Register segfault handler in a way that a core dump is generated as well
    struct sigaction sa;

    sa.sa_flags = SA_RESETHAND;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_handler;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        LOG_ERROR << "Couldn't register segfault handler on SIGSEGV";
    }
    if (sigaction(SIGABRT, &sa, NULL) == -1) {
        LOG_ERROR << "Couldn't register segfault handler on SIGABRT";
    }

    std::set_terminate(terminate_handler);
}

int Engine::run(int use_instance, bool docker, System *system){
    return EXIT_SUCCESS;
}

bool System::init(int argc, char *argv[]) {
    register_handlers();

    if (!init_cli(argc, argv)) {
        return false;
    }
    
    init_logger();

    FLAGS_stderrthreshold = 1; // Display warnings by caffe in stderr
    FLAGS_minloglevel = 1;   // Only log warnings and errors
    google::InitGoogleLogging(argv[0]);

    return true;
}

int System::run() {
    // use this instance for the backend
    int use_instance = cli["instance"].as<int>();
    LOG_INFO << "connect to JRLSoccer Backend with instance " << use_instance;

    LOG_INFO << "-----------------------------------------------------";
    LOG_INFO << "Starting the frontend!";
    LOG_INFO << "-----------------------------------------------------";

    Instance inst(use_instance);

    if (!inst.is_unique()) {
        LOG_INFO << "There is already an instance of this process!";
        return EXIT_FAILURE;
    }

    return engine->run(use_instance, docker, this);
}

void System::stop() {
    xlog->stop();
}

void System::init_logger() {
    xlog = std::make_unique<XLogger>(LOGID);
    xlog_console = new ConsoleBackend(LOGID);

    // set fancy log-lvl descs
    xlog->set_loglvl_desc(10, "\e[1;31m[E]\e[0m");
    xlog->set_loglvl_desc(7,  "\e[1;33m[W]\e[0m");
    xlog->set_loglvl_desc(5,  "\e[1;37m[i]\e[0m");
    xlog->set_loglvl_desc(3,  "\e[1;36m[D]\e[0m");
    xlog->set_loglvl_desc(1,  "\e[1;34m[P]\e[0m");

    xlog->add_backend(xlog_console, "%%FANCYLVL%% %%MSG%%\n");

    // minimum runtime-log-lvl set here!
    // minimum compile-log-lvl as #define inside c/u/tafel.h
    xlog->set_min_loglvl(3);

    // do not strip whitespaces before/after msg!
    xlog->set_msg_stripping(false);

    LOG_INFO << "Started logger (" << LOGID << ") with min-lvls ["
            << "runtime = " << xlog->get_min_loglvl() << " "
            << "compile-time = " << XLOG_MIN_LOG_LVL << "]";
}

bool System::init_cli(int argc, char *argv[]) {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message.")
    ("stepwise,S", "run framework in stepwise mode")
    ("instance,i", boost::program_options::value<int>()->default_value(naoqiBackendPort),
    "connect to a backend with the given instance.")
    ("simulatorHost,s", boost::program_options::value<std::string>(), "Host ip v4 address of the Simulator")
    ("docker", "simulation mode inside docker container. Reads additional options from environment variables.")
    ("data,g", boost::program_options::value<std::string>(),
    "used ground truth directory.")
    ("buildinfo", "print buildinfo");

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), cli);
    boost::program_options::notify(cli);

    if (cli.count("help")) {
        std::cout << "Team Bembelbots, JRL Soccer Framework V2." << std::endl;
        std::cout << desc << std::endl;
        return false;
    }

    if (cli.count("buildinfo")) {
        std::cout << readBuildInfo();
        return EXIT_SUCCESS;
    }

    if (cli.count("docker")) {
        docker = true;
        std::cout << "Framework running in Docker mode." << std::endl;
    }

    return true;
}
