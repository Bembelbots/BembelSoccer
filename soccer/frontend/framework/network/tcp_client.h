#pragma once

#include "network.h"

using boost::asio::ip::address;
using boost::asio::ip::address_v4;
using boost::asio::ip::udp;
using boost::asio::ip::tcp;

/**
 * @brief connects to a TCP server
 */
class TCPClient : Network {
    public:
        /**
         * sending only constructor, resulting object will not be able to receive any messages
         * @param server remote endpoint to connect to
         */
        TCPClient(const address &address, const NetworkPorts &port, const bool &asyncConnect=true, const bool &nodelay=false,  const size_t &recvBufSize=0, const size_t sendBufSize=0);

        TCPClient(const TCPClient&) = delete;
        TCPClient& operator=(const TCPClient&) = delete;
        
        virtual ~TCPClient();

        /**
         * send messages to server
         * @param msg message to send
         * @param size size of message in bytes
         */
        void send(const char *msg, const size_t size);

        /**
         * receive data (blocking)
         * @param buf receive buffer
         * @param size size of receive buffer in bytes
         * @return number of bytes read
         */
        size_t recv_block(char *buf, const size_t size);

        size_t available() { return _socket->available(); };


    private:
        tcp::socket *_socket;   ///< tcp socket
        //recv_func_t _recv_func;

        /// handler for async connecting
        void handle_connect(const boost::system::error_code &error); // cppcheck-suppress functionStatic
    
        /// handler for async write
        void handle_write(const boost::system::error_code& error, size_t size); // cppcheck-suppress functionStatic
};

// vim: set ts=4 sw=4 sts=4 expandtab:
