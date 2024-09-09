#include "whistle.h"
#include "commands.h"
#include "detect/alsarecorder.h"
#include "detect/whistle_detector.h"
#include "whistle_commands_generated.h"
#include "whistle_message_generated.h"
#include <framework/util/clock.h>
#include <framework/thread/util.h>

static constexpr TimestampMs RECORD_ONLY_TIME = 5000;
static constexpr size_t MIN_WHISTLE_LENGTH = 600;
static constexpr float WHISTLE_THRESHOLD = 2500;

void Whistle::setup() {
    recorder = std::make_shared<AlsaRecorder>(AlsaRecorder::V5_SETTINGS_41000_STEREO);
    detector = std::make_shared<WhistleDetector>(recorder.get(), MIN_WHISTLE_LENGTH, WHISTLE_THRESHOLD);
    cmds.connect<bbapi::WhistleRecordOnlyT, &Whistle::onRecordOnly>(this);
    cmds.connect<bbapi::WhistleStartT, &Whistle::onStart>(this);
    cmds.connect<bbapi::WhistleStopT, &Whistle::onStop>(this);
}

void Whistle::connect(rt::Linker &link) {
    link.name = "Whistle";
    link(settings);
    link(cmds);
    link(event);
}

void Whistle::stop() {
    if(listening) {
        detector->stop();
    }
}

void Whistle::process() {
    if (!listening) {
        sleep_for(1ms);
        return;
    }

    bbapi::WhistleMessageT result;
    result.found = detector->process();

    if (prevFound != result.found)
        event.emit(result);
    prevFound = result.found;
}

void Whistle::onRecordOnly(bbapi::WhistleRecordOnlyT &) {
    if(!listening) {
        listening = true;
        detector->start();
    }
    detector->onlyRecord();
}

void Whistle::onStart(bbapi::WhistleStartT &) {
    if(listening || settings->simulator) {
        return;
    }
    listening = true;
    detector->start();
}

void Whistle::onStop(bbapi::WhistleStopT &) {
    if(!listening) {
        return;
    }
    listening = false;
    detector->stop();
}
