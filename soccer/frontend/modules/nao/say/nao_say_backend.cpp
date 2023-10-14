//
// Created by kito on 4/24/18.
//

#include <mutex>
#include "nao_say_backend.h"
#include <framework/logger/logger.h>

#if V6
#include <representations/blackboards/settings.h>
#include <libbembelbots/tts.hpp>
#else
#include <libbembelbots/bbcommands.h>
#endif


NaoSayBackend::NaoSayBackend(const std::string &my_id, const int &instance, bool simulator)
        : BaseLoggerBackend(my_id, "naosay"), instance(instance), simulator(simulator), naoSayTick(0) {
#if !V6
    std::stringstream shmNameCR;
    shmNameCR << bembelbotsShmName << "_" << instance;
    LOG_INFO << "opening shm with name " << shmNameCR.str();
    shm = std::make_unique<BembelbotsShm>(shmNameCR.str(), false);
#endif
}

void NaoSayBackend::write(const std::string &msg) {
    static std::string last_msg;
    static std::mutex say_mutex;
    {
        std::lock_guard<std::mutex> guard(say_mutex);
        if (msg == last_msg) {
            return;
        }
        last_msg = msg;
    }

#if V6
	// V6: use fifo to mimic
	static TTSFifo tts;
    if (!simulator)
        tts.say(msg);
#else
	// V5: send msg to backend which calls naoqi say
    BBFunctionSay say_cmd(msg);
    say_cmd.tick = ++naoSayTick;

    (*shm)->control.consumedData() = say_cmd;
    (*shm)->control.consume();
#endif

    LOG_DEBUG << "[NaoSay]: " << msg;
}
