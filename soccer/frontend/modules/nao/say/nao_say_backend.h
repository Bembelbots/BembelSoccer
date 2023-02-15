//
// Created by kito on 4/24/18.
//
#pragma once

#include <string>
#include <memory>
#include <framework/logger/backends.h>
#include <libbembelbots/bembelbots_shm.h>

class NaoSayBackend : public BaseLoggerBackend {
public:
    explicit NaoSayBackend(const std::string &my_id, const int &instance, bool simulator);
    void write(const std::string &msg);

private:
    std::unique_ptr<BembelbotsShm> shm{nullptr};
    int instance;
    bool simulator{false};
    int naoSayTick;
};
