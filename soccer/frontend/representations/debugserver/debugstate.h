#pragma once
#include <framework/network/network.h>

using UdpEndpoint = boost::asio::ip::udp::endpoint;

struct DebugState {
    UdpEndpoint debug_ep;
};
