#include <thread>

#include "network.h"
#include "../thread/threadmanager.h"
#include <iostream>

std::shared_ptr<NetworkIO> Network::_io = nullptr;

NetworkIO::NetworkIO() :
   _work(new boost::asio::io_service::work(_io_service))
{
    CreateNetworkThread(this);
}

NetworkIO::~NetworkIO() {
    if(_work != nullptr) {
        delete _work;
        _work = nullptr;
    }
}

void NetworkIO::worker(ThreadContext *context) {
    context->notifyReady();
    _io_service.run();
}

void NetworkIO::stop() { 
    _io_service.stop();
}

boost::asio::io_service *NetworkIO::getIOService() {
    return &_io_service;
}


// vim: set ts=4 sw=4 sts=4 expandtab:
