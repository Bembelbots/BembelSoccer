#include <cstring>
#include <iostream>
#include <memory>

#include "tcp-server.h"

using namespace boost::asio;

TCPSession::TCPSession(tcp::socket socket, const bool &nodelay, const size_t &recvBufSize, const size_t &sendBufSize)
  : _socket(std::move(socket)) {
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
    } catch (std::exception &e) {
        std::cerr << __func__ << ": " << e.what() << std::endl;
    }
}

bool TCPSession::is_open() const {
    return _socket.is_open();
}

void TCPSession::write(const shared_const_buffer &buf) {
    try {
        boost::asio::async_write(_socket, buf, [&](boost::system::error_code ec, std::size_t /*length*/) {
            if (ec)
                close();
        });
    } catch (std::exception &e) {
        close();
    }
}

TCPServer::TCPServer(
        const NetworkPorts &port, const bool &nodelay, const size_t &recvBufSize, const size_t &sendBufSize)
  : nodelay(nodelay), recvBufSize(recvBufSize), sendBufSize(sendBufSize) {
    if (port == Network::SPL_MSG)
        throw std::runtime_error("SPL_MSG port may not be used with NetworkTCPServer");

    _socket = new tcp::socket(*_io->getIOService());
    _acceptor = new tcp::acceptor(*_io->getIOService(), tcp::endpoint(tcp::v6(), port));
    accept();
}

TCPServer::~TCPServer() {
    for (auto &s : _sessions) {
        s->close();
        s.reset();
    }
}

void TCPServer::accept() {
    _acceptor->async_accept(*_socket, [&](boost::system::error_code ec) {
        if (!ec)
            _sessions.push_back(std::make_shared<TCPSession>(std::move(*_socket), nodelay, recvBufSize, sendBufSize));
        accept();
    });
}

void TCPServer::write(const char *msg, const size_t size) {
    // remove dead sessions
    _sessions.remove_if([](const std::shared_ptr<TCPSession> &s) { return !s->is_open(); });

    shared_const_buffer buf(msg, size);

    for (auto &s : _sessions)
        s->write(buf);
}

// vim: set ts=4 sw=4 sts=4 expandtab:
