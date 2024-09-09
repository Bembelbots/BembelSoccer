#pragma once

#include <list>
#include <memory>

#include "network.h"
#include "tcp_session.h"
#include "buffer.h"

using boost::asio::ip::address;
using boost::asio::ip::address_v4;
using boost::asio::ip::udp;
using boost::asio::ip::tcp;

/**
 * @brief minimal TCP server implementation, always listens on 0.0.0.0, only supports sending data
 */
class TCPServer : Network {
    public:
        explicit TCPServer(const NetworkPorts &port, const bool &nodelay=false, const size_t &recvBufSize=0, const size_t &sendBufSize=0);
        
        virtual ~TCPServer();

        TCPServer(const TCPServer&) = delete;
        TCPServer& operator=(const TCPServer&) = delete;

        void write(const char *msg, const size_t size);

        bool is_busy() const;

    private:
        tcp::socket *_socket;
        tcp::acceptor *_acceptor;
        std::list< std::shared_ptr<TCPSession> > _sessions;
        bool nodelay;
        size_t recvBufSize, sendBufSize;

        void accept();
};


// vim: set ts=4 sw=4 sts=4 expandtab:
