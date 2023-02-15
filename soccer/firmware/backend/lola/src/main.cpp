#include <csignal>
#include <cstddef>

#include <chrono>
#include <string>
#include <thread>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <lola/connector.h>

#include <framework/util/buildinfo.hpp>

using boost::asio::ip::address_v4;
using boost::asio::ip::tcp;
using boost::asio::local::stream_protocol;
using namespace std::chrono_literals;
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

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message.")
            ("tcp,t", "use TCP instead of UNIX socket.")
            ("endpoint,e", po::value<std::string>(), "specify connection endpoint (UNIX: <socket path>, TCP: <port> or <ip:port>)")
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

    if (vm.count("tcp")) {
        // use TCP socket
        tcp::endpoint tcp_ep(address_v4::loopback(), 10000);

        // parse TCP endpoint
        if (vm.count("endpoint")) {
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
    c->run();
    unstiff(s);

    delete c;
    return 0;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
