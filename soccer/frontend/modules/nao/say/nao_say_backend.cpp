//
// Created by kito on 4/24/18.
//

#include <mutex>
#include "nao_say_backend.h"

#include <libbembelbots/tts.hpp>
#include <framework/logger/logger.h>
#include <representations/blackboards/settings.h>


NaoSayBackend::NaoSayBackend(const std::string &my_id, const int &instance, bool simulator)
        : BaseLoggerBackend(my_id, "naosay"), tts(new TTSSpeechd(my_id.c_str())), instance(instance), simulator(simulator), naoSayTick(0) {
}

void NaoSayBackend::write(const std::string &msg) {
    if (msg == last_msg.load())
            return;
    last_msg.store(msg);

    if (!simulator)
        tts->say(msg);
    LOG_DEBUG << "[NaoSay]: " << msg;
}
