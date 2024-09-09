#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/lockfree/queue.hpp>
#include <deque>
#include <list>
#include <memory>

#include "network.h"
#include "buffer.h"

using boost::asio::ip::address;
using boost::asio::ip::address_v4;
using boost::asio::ip::udp;
using boost::asio::ip::tcp;

class TCPSession : public std::enable_shared_from_this<TCPSession> {
    public:
        TCPSession(tcp::socket socket, boost::asio::io_service &io, const bool &nodelay, const size_t &recvBufSize, const size_t &sendBufSize);
        
        virtual ~TCPSession();
        
        void close();
        bool is_open() const;
        bool is_busy() const;

        void write(const shared_const_buffer &buf);

    private:
        tcp::socket _socket;
        boost::asio::io_service::strand _strand;
        std::deque<shared_const_buffer> _queue;

        void addWrite(const shared_const_buffer &buf);
        void doWrite();
        void writeHandler(boost::system::error_code ec, std::size_t length);
};


// vim: set ts=4 sw=4 sts=4 expandtab:
