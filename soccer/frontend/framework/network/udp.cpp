#include <mutex>
#include <cstring>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

#include "udp.h"
#include "buffer.h"

using lock = std::lock_guard<std::mutex>;

// I'd tell you a joke about UDP but you wouldn't get it.
UDP::UDP(const NetworkPorts &port, const int &tn) : teamNumber(tn) {
    bindSocket(port);
}

UDP::UDP(const NetworkPorts &port, recv_func_t fn, const int &tn) : _recv_func(fn), teamNumber(tn) {
    bindSocket(port);
}

void UDP::bindSocket(const NetworkPorts &port) {
    memset(_recv_buf, 0, sizeof(_recv_buf));
    openSocket();
    address_v4 a;
    a = address_v4::any();
    udp::endpoint listen_endpoint(a, translatePort(port));
    _socket->bind(listen_endpoint);
}

void UDP::openSocket() {
    // only create _io object once, otherwise we would spawn a new thread for every
    // network object
    _socket = new udp::socket(*(_io->getIOService()));
    _socket->open(udp::v4());
    _socket->set_option(udp::socket::reuse_address(true));
}

UDP::~UDP() {
    delete _socket;
}

void UDP::sendTo(const char *msg, const size_t &size, const udp::endpoint &ep) {
    _socket->set_option(boost::asio::socket_base::broadcast(false)); // disable broadcast flag on socket
    _sendEP = ep;
    shared_const_buffer buf(msg, size);
    _socket->async_send_to(buf, _sendEP, boost::bind(&UDP::handle_send_to, this, boost::asio::placeholders::error));
}

void UDP::bcast(const char *msg, const size_t &size, const NetworkPorts &port) {
    _socket->set_option(boost::asio::socket_base::broadcast(true)); // enable broadcast flag on socket
    _sendEP.address(address_v4::broadcast());
    _sendEP.port(translatePort(port));
    shared_const_buffer buf(msg, size);
    _socket->async_send_to(buf, _sendEP, boost::bind(&UDP::handle_send_to, this, boost::asio::placeholders::error));
}

void UDP::triggerAsyncReceive() {
    try {
        _socket->async_receive_from(boost::asio::buffer(_recv_buf, RECV_BUF_SIZE),
                _recvEP,
                boost::bind(&UDP::handle_receive_from,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    } catch (std::exception &e) {
        std::cerr << "error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
    }
}

void UDP::handle_receive_from(const boost::system::error_code & /*error*/, size_t bytes_recvd) {
    // use mutex to avoid "overlapping" callbacks in case a new packet arrives while _recv_func
    // is still processing
    lock l(_recv_mtx);
    _recv_func(_recv_buf, bytes_recvd, _recvEP);
    triggerAsyncReceive();
}

void UDP::handle_send_to(const boost::system::error_code &error) {
    if (error and error.value() != boost::system::errc::network_unreachable)
        std::cerr << "error sending:" << error.message() << std::endl;
}

int UDP::translatePort(const NetworkPorts &port) const {
    if (port == SPL_MSG) {
        if (teamNumber < 0)
            throw std::runtime_error("team number for SPL message port not set");
        return 10000 + teamNumber;
    } else {
        return port;
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:
