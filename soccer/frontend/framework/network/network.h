#pragma once

#include <memory>
#include <boost/asio.hpp>

class ThreadContext;

// forward declaration
namespace std {
    class thread;
}

class NetworkIO;
extern void CreateNetworkThread(NetworkIO *network);

/**
 * @brief wrapper for boost::asio::io_service. All network objects will use a single instance of this class
 */
class NetworkIO {
    public:
        /// default constructor, starts a new thread in which io_service.run() will be executed
        NetworkIO();

        virtual ~NetworkIO();

        NetworkIO(const NetworkIO&) = delete;
        NetworkIO& operator=(const NetworkIO&) = delete;

        void stop();

        /// getter for pointer to the internal io_service
        boost::asio::io_service *getIOService();

    private:
        friend void CreateNetworkThread(NetworkIO *network);
        boost::asio::io_service _io_service;
        boost::asio::io_service::work *_work;   ///< this ensures that io_service will keep running, even if it's currently out of work (see boost docs)
        void worker(ThreadContext *);
};

/**
 * @brief common base class for all network classes.
 */
class Network {
    public:
        /**
         * Network port number assignments
         */
        enum NetworkPorts {
            SPL_MSG     = -1,   // 10000 + teamNumber according to 2015 rules
            RANDOM      = 0,
            GAMECONTROL = 3838,
            DEBUG       = 10350,
            MONITOR     = 10399,
            TCP_TCAM    = 10001,
            TCP_BCAM    = 10002,
        };

        static void set_network_io(std::shared_ptr<NetworkIO> io) {
            _io = std::move(io);
        }

        static void reset_network_io() {
            _io.reset();
        }

    protected:
        static std::shared_ptr<NetworkIO> _io;  ///< holds io_service, so all network objects all use a single io_service.
};

// vim: set ts=4 sw=4 sts=4 expandtab:
