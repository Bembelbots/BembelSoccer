#include <boost/asio/buffer.hpp>
#include <cstring>
#include <iostream>

#include <boost/bind.hpp>

#include "tcp_client.h"
#include "buffer.h"

using namespace boost::asio;

TCPClient::TCPClient(const address &address, const NetworkPorts &port, const bool &asyncConnect, const bool &nodelay,  const size_t &recvBufSize, const size_t sendBufSize) {
    tcp::endpoint server(address, port);
    
    _socket = new tcp::socket(*(_io->getIOService()));

    if (asyncConnect)
        _socket->async_connect(server, boost::bind(
                        &TCPClient::handle_connect,
                        this, boost::asio::placeholders::error));
    else
        _socket->connect(server);


    _socket->set_option(ip::tcp::no_delay(nodelay));
    if (recvBufSize > 0)
        _socket->set_option(socket_base::receive_buffer_size(recvBufSize));
    if (sendBufSize > 0)
        _socket->set_option(socket_base::send_buffer_size(sendBufSize));
}


TCPClient::~TCPClient() {
    try {
        _socket->cancel();
        _socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        _socket->close();
    } catch(std::exception &e) {
        std::cerr << "error while disconnecting tcp socket: " << e.what() << std::endl;
    } catch(...) {
        std::cerr << "unknown error while disconnecting tcp socket." << std::endl;
    }
    delete _socket;
}


void TCPClient::send(const char *msg, const size_t size) {
    try {
        _socket->async_send(shared_const_buffer(msg, size),
              boost::bind(&TCPClient::handle_write, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
    } catch (std::exception &e) {
        std::cerr << "error on TCP send: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown error on TCP send." << std::endl;
    }
}


size_t TCPClient::recv_block(char *buf, const size_t size) {
    return _socket->read_some(boost::asio::buffer(buf, size));
}

void TCPClient::handle_connect(const boost::system::error_code &error) {
    if (error)
        std::cerr << "error on TCP connect: " << error.message() << std::endl;
}


void TCPClient::handle_write(const boost::system::error_code& error, size_t /*size*/) {
    if (error)
        std::cerr << "error on TCP write: " << error.message() << std::endl;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
