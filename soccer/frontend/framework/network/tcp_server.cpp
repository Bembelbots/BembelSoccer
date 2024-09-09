#include <memory>
#include <cstring>
#include <iostream>

#include "tcp_server.h"

using namespace boost::asio;

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
            _sessions.push_back(std::make_shared<TCPSession>(
                    std::move(*_socket), *_io->getIOService(), nodelay, recvBufSize, sendBufSize));
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

bool TCPServer::is_busy() const {
    bool busy{false};
    for (auto &s : _sessions)
        busy |= s->is_busy();

    return busy;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
