#include <boost/asio/io_service.hpp>
#include <memory>
#include <cstring>
#include <iostream>

#include <boost/bind/bind.hpp>

#include "tcp_server.h"
#include "network.h"

using namespace boost::asio;

TCPSession::TCPSession(tcp::socket socket, boost::asio::io_service &io, const bool &nodelay, const size_t &recvBufSize,
        const size_t &sendBufSize)
  : _socket(std::move(socket)), _strand(io) {
    _socket.set_option(ip::tcp::no_delay(nodelay));
    if (recvBufSize > 0)
        _socket.set_option(socket_base::receive_buffer_size(recvBufSize));
    if (sendBufSize > 0)
        _socket.set_option(socket_base::send_buffer_size(sendBufSize));
}

TCPSession::~TCPSession() {
    close();
}

void TCPSession::close() {
    try {
        try {
            _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        } catch (...) {
            // don't care about errors here
        }
        _socket.close();
        _queue.clear();
    } catch (std::exception &e) {
        std::cerr << __func__ << ": " << e.what() << std::endl;
    }
}

bool TCPSession::is_open() const {
    return _socket.is_open();
}

bool TCPSession::is_busy() const {
    return !_queue.empty();
}

void TCPSession::write(const shared_const_buffer &buf) {
    _strand.post(boost::bind(&TCPSession::addWrite, this, buf));
}

void TCPSession::addWrite(const shared_const_buffer &buf) {
    _queue.push_back(buf);

    // queue was empty, start sending data
    if (_queue.size() < 2)
        doWrite();
}

void TCPSession::doWrite() {
    auto &buf = _queue.front();
    try {
        boost::asio::async_write(_socket,
                buf,
                _strand.wrap(boost::bind(&TCPSession::writeHandler,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
    } catch (std::exception &e) {
        close();
    }
}

void TCPSession::writeHandler(boost::system::error_code ec, std::size_t length) {
    _queue.pop_front();
    if (ec)
        close();

    // continue sending until queue is empty
    if (!_queue.empty())
        doWrite();
}

// vim: set ts=4 sw=4 sts=4 expandtab:
