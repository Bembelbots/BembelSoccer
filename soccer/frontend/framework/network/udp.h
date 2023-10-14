#pragma once

#include <mutex>
#include <functional>

#include "network.h"

using boost::asio::ip::address;
using boost::asio::ip::address_v4;
using boost::asio::ip::udp;
using boost::asio::ip::tcp;

// typedef for receive callbacks
using recv_func_t = std::function<void(const char *, const size_t &, const udp::endpoint &)>;

// forward declaration
class NetworkIO;

/**
 * @brief handles UDP communication
 */
class UDP : Network {
public:
    /**
     * UDP constructor only for sending.
     * Resulting object will not receive anything.
     * @param port is a port number from which sent packets will originate
     * @param tn team number used for SPL message port
     */
    UDP(const NetworkPorts &port = RANDOM, const int &tn = -1);

    /**
     * lambda compatible constructor
     * @param port is a port number from which sent packets will originate
     * @param fn callback function
     * @param tn team number used for SPL message port
     */
    UDP(const NetworkPorts &port, recv_func_t fn, const int &tn = -1);

    /**
     * UDP constructor for sending & receiving.
     * @param port is a port number from which sent packets will originate
     * @param bindBcast determines whether socket will bind to broadcast IP or any IP in simulator (real Nao will always bind any IP)
     * @param f callback function
     * @param object pointer to the object from which the callback function will be called
     * @param tn team number used for SPL message port
     */
    template<class T, class O>
    UDP(const NetworkPorts &port, void (T::*f)(const char *, const size_t &, const udp::endpoint &), const O &object, const int &tn = -1)
        : _recv_func(std::bind(f, object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
        , teamNumber(tn) {
        bindSocket(port);
        triggerAsyncReceive();
    }

    virtual ~UDP();

    /**
     * send messages to a specific IP/port
     * @param msg message to send
     * @param size size of the message in bytes
     * @param ep udp endpoint to which the message will be sent
     */
    void sendTo(const char *msg, const size_t &size, const udp::endpoint &ep);

    /**
     * sends messages using UDP broadcasts
     * @param msg message to send
     * @param size size of the message in bytes
     * @param port determines on which port the message will be broadcast
     */
    void bcast(const char *msg, const size_t &size, const NetworkPorts &port);

private:
    udp::socket *_socket;           ///< pointer to UDP socket
    udp::endpoint _sendEP;          ///< target for send operations
    udp::endpoint _recvEP;          ///< source from whom last message was received
    recv_func_t _recv_func;         ///< callback for receiving
    std::mutex _recv_mtx;           ///< mutex for locking receive callbacks
    enum { RECV_BUF_SIZE = 66000 };
    char _recv_buf[RECV_BUF_SIZE];  ///< receive buffer (slightly larger than max. UDPv4 packet size)
    int teamNumber;                 ///< team number used for SPL standard messages port

    /// helper method to open the socket, used by all constructors
    void openSocket();

    /// helper method to bind socket (used by receiving constructor)
    void bindSocket(const NetworkPorts &port);

    /// trigger async receiving of next packet
    void triggerAsyncReceive();

    /// handler for receiving packets (calls callback _recv_func)
    void handle_receive_from(const boost::system::error_code &error, size_t bytes_recvd);

    /// handler for async sending
    void handle_send_to(const boost::system::error_code &error); // cppcheck-suppress functionStatic

    int translatePort(const NetworkPorts &port) const;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
