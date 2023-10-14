#include <csignal>
#include <cstddef>

#include <chrono>
#include <string>
#include <thread>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <lola/connector.h>

#include <framework/simple_application.h>
#include <framework/util/assert.h>
#include <framework/util/getenv.h>
#include <framework/util/buildinfo.hpp>

using boost::asio::ip::address_v4;
using boost::asio::ip::tcp;
using boost::asio::local::stream_protocol;
using namespace std::chrono_literals;
using std::this_thread::sleep_for;
namespace po = boost::program_options;

static lola::Connector *c;

static void unstiff(lola::Connector::socket_t &sock) {
    if (!sock.is_open())
        return;

    auto buffer{lola::Connector::stiffnessRequest(-1.f)};

    // wait for start of a new LoLa cycle
    char buf[1000];
    sock.receive(boost::asio::buffer(buf));

    sock.send(boost::asio::buffer(buffer.data(), buffer.size()));
    std::cout << "LoLA connector stopped, unstiff all joints" << std::endl;
}

// ctrl-c signal handler
static void sigHandler(int sig) {
    c->stop();
}

int main(int argc, char **argv) {
    boost::asio::io_service io_service;
    lola::Connector::socket_t s{io_service};
    lola::Connector::endpoint_t ep;
    static const std::string SOCK_PATH{"/tmp/robocup"};

    XLogger logger("lola-backend");
    CreateXLoggerThread(&logger);

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message.")
            ("tcp,t", "use TCP instead of UNIX socket.")
            ("endpoint,e", po::value<std::string>(), "specify connection endpoint (UNIX: <socket path>, TCP: <port> or <ip:port>)")
            ("docker", "simulation mode inside docker container. Reads additional options from environment variables.")
            ("buildinfo", "print buildinfo");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    // just print buildinfo and exit
    if (vm.count("buildinfo")) {
        std::cout << readBuildInfo();
        return 0;
    }

    if (vm.count("tcp") || vm.count("docker")) {
        // use TCP socket
        tcp::endpoint tcp_ep(address_v4::loopback(), 10000);

        if (vm.count("docker")) {
            // use docker environment variables
            const std::string lola_port{getEnvVar("LOLA_PORT")},
                  sim_host{getEnvVar("SIMULATOR_HOST")};
            jsassert(!lola_port.empty()) << "set LOLA_PORT environment variable for docker mode!";
            jsassert(!sim_host.empty()) << "set SIMULATOR_HOST environment variable for docker mode!";
            int port = std::stoi(lola_port);
            tcp_ep = tcp::endpoint(address_v4::from_string(sim_host), port);
            
        } else if (vm.count("endpoint")) {
            // parse TCP endpoint
            std::string ep_string{vm["endpoint"].as<std::string>()};
            auto i = ep_string.find_first_of(':');
            if (i == std::string::npos) {
                // just a port specified
                tcp_ep.port(std::stoi(ep_string));
            } else {
                // IP:PORT specified
                std::string ip{ep_string.substr(0, i)};
                int port{std::stoi(ep_string.substr(i + 1))};
                tcp_ep = tcp::endpoint(address_v4::from_string(ip), port);
            }
        }

        ep = tcp_ep;
    } else {
        // use UNIX socket
        std::string path{SOCK_PATH};
        if (vm.count("endpoint")) // use alternate socket
            path = vm["endpoint"].as<std::string>();

        ep = stream_protocol::endpoint(path);
    }

    signal(SIGTERM, sigHandler);
    signal(SIGINT, sigHandler);

    c = new lola::Connector(s, ep);

    std::cout << "Running LoLA connector" << std::endl;
    bool shutdown = c->run();
    unstiff(s);
    delete c;

    if (shutdown) {
        sleep_for(2s); // sleep for TTS :/
        system("/usr/bin/sudo /usr/sbin/poweroff");
    }

    return 0;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
