//
// Created by kito on 4/24/18.
//
#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <string_view>
#include <framework/logger/backends.h>

class TTSSpeechd;

class NaoSayBackend : public BaseLoggerBackend {
public:
    explicit NaoSayBackend(const std::string &my_id, const bool simulator);
    void write(const std::string &msg);

private:
    std::unique_ptr<TTSSpeechd> tts{nullptr};
    std::atomic<std::string_view> last_msg;
    bool simulator{false};
    int naoSayTick;
};
