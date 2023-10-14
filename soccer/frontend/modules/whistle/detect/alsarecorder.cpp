/*!
 * \brief An interface to ALSA for recording sound data.
 * \author Thomas Hamboeck, Austrian Kangaroos 2014
 * \author Felix Weiglhofer
 */


#include "alsarecorder.h"

#include <framework/logger/logger.h>
#include <framework/util/assert.h>

#include <cmath>


#define CATCH_ALSA(call) \
    err = call; \
    LOG_ERROR_IF(err < 0) << "AlsaRecorder:" << __LINE__ << ": " \
                          << snd_strerror(err)

const AlsaRecorder::Settings AlsaRecorder::V5_SETTINGS = {
    2048,
    1,
    12000,
    "default",
    "default",
    "Capture",
    100
};

const AlsaRecorder::Settings AlsaRecorder::V5_SETTINGS_41000_STEREO = {
    1024,
    2,
    44100,
    "default",
    "default",
    "Capture",
    100
};


AlsaRecorder::AlsaRecorder(const Settings &s)
    : settings(s)
    , audioBuffer(nullptr)
    , captureHandle(nullptr) {
    createAudioBuffer();
}

AlsaRecorder::~AlsaRecorder() {
    destroyAlsa();
    delete[] audioBuffer;
}

void AlsaRecorder::createAudioBuffer() {
    jsassert(settings.channelNum > 0);
    jsassert(settings.sampleRate > 0);
    audioBuffer = new int16_t[getChannelNum() * getBufferSize()];
}

void AlsaRecorder::stop() {
    jsassert(captureHandle != nullptr);
    destroyAlsa();
}

bool AlsaRecorder::fetch() {
    jsassert(captureHandle != nullptr);
    jsassert(audioBuffer != nullptr);

    /* snd_pcm_state_t state = snd_pcm_state(captureHandle); */

    /* switch (state) { */
    /* case SND_PCM_STATE_PREPARED: */
    /*     LOG_DEBUG_EVERY_N(10) << "Alsa state: prepared"; */
    /*     break; */
    /* case SND_PCM_STATE_RUNNING: */
    /*     LOG_DEBUG_EVERY_N(10) << "Alsa state: running"; */
    /*     break; */
    /* case SND_PCM_STATE_XRUN: */
    /*     LOG_ERROR << "pcm xrun detected."; */
    /*     break; */
    /* case SND_PCM_STATE_DRAINING: */
    /*     LOG_DEBUG_EVERY_N(10) << "Alsa state: draining"; */
    /*     CATCH_ALSA(snd_pcm_prepare(captureHandle)); */
    /*     break; */
    /* case SND_PCM_STATE_PAUSED: */
    /*     LOG_DEBUG_EVERY_N(10) << "Alsa state: paused"; */
    /*     CATCH_ALSA(snd_pcm_pause(captureHandle, 0)); */
    /*     break; */
    /* default: */
    /*     LOG_ERROR << "Unhandled pcm state detected."; */
    /* } */

    while (true) {
        int err;
        if ((err = snd_pcm_readi(captureHandle, audioBuffer,
                                 getBufferSize())) == static_cast<int>(getBufferSize())) {
            return true;
        }
        LOG_ERROR << "read from audio interface failed " << snd_strerror(err);
        /* try to recover */
        if ((err = xrunRecovery(captureHandle, err)) !=  0) {
            LOG_ERROR << "couldn't recover " << snd_strerror(err);
            return false;
        }
    }
}

int16_t *AlsaRecorder::getBuffer() {
    return audioBuffer;
}

size_t AlsaRecorder::getBufferSize() {
    return settings.bufferSize;
}

short AlsaRecorder::getChannelNum() {
    return settings.channelNum;
}

double AlsaRecorder::getSampleRate() {
    return settings.sampleRate;
}

/*******************************************************************/
/**
 * Must be called before reading data.
 */
void AlsaRecorder::start() {
    jsassert(captureHandle == nullptr);

    int err;
    snd_pcm_hw_params_t *hwParams;

    if ((err = snd_pcm_open(&captureHandle, settings.soundDevice.c_str(),
                            SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        LOG_ERROR << "cannot open audio device " << settings.soundDevice << "(" <<
                  snd_strerror(err) << ")";
        return;
    }

    if ((err = snd_pcm_hw_params_malloc(&hwParams)) < 0) {
        LOG_ERROR << "cannot allocate hardware parameter structure " << snd_strerror(
                      err);
        return;
    }

    if ((err = snd_pcm_hw_params_any(captureHandle, hwParams)) < 0) {
        LOG_ERROR << "cannot initialize hardware parameter structure " << snd_strerror(
                      err);
        return;
    }

    if ((err = snd_pcm_hw_params_set_access(captureHandle, hwParams,
                                            SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        LOG_ERROR << "cannot set access type " << snd_strerror(err);
        return;
    }

    if ((err = snd_pcm_hw_params_set_format(captureHandle, hwParams,
                                            SND_PCM_FORMAT_S16_LE)) < 0) {
        LOG_ERROR << "cannot set sample format " << snd_strerror(err);
        return;
    }

    unsigned int oSR;
    oSR = getSampleRate();
    if ((err = snd_pcm_hw_params_set_rate_near(captureHandle, hwParams, &oSR,
               0)) < 0) {
        LOG_ERROR << "cannot set sample rate " << snd_strerror(err);
        return;
    }

    if (oSR != getSampleRate()) {
        LOG_ERROR << "cannot set sample rate, correct should be: " << getSampleRate()
                  << ", is " << oSR;
        return;
    }

    LOG_DEBUG << "ALSA-RX opened with a samplerate of " << oSR << " (requested: "
              << getSampleRate() << ").";

    if ((err = snd_pcm_hw_params_set_channels(captureHandle, hwParams,
               getChannelNum())) < 0) {
        LOG_ERROR << "cannot set channel count " << snd_strerror(err);
        return;
    }

    /* constexpr int hwBufMultiple = 10; */
    /* CATCH_ALSA(snd_pcm_hw_params_set_buffer_size( */
    /*             captureHandle, */
    /*             hwParams, */
    /*             getBufferSize() * hwBufMultiple)); */
    /* if (err < 0) return; */

    /* snd_pcm_uframes_t hwbuf; */
    /* CATCH_ALSA(snd_pcm_hw_params_get_buffer_size_min(hwParams, &hwbuf)); */
    /* LOG_DEBUG << "audio capture: hw_buffer_size_min = " << hwbuf; */

    /* CATCH_ALSA(snd_pcm_hw_params_get_buffer_size_max(hwParams, &hwbuf)); */
    /* LOG_DEBUG << "audio capture: hw_buffer_size_max = " << hwbuf; */

    /* CATCH_ALSA(snd_pcm_hw_params_get_buffer_size(hwParams, &hwbuf)); */
    /* LOG_DEBUG << "audio capture: hw_buffer_size = " << hwbuf; */

    if ((err = snd_pcm_hw_params(captureHandle, hwParams)) < 0) {
        LOG_ERROR << "cannot set parameters " << snd_strerror(err);
        return;
    }

    bool canPause = snd_pcm_hw_params_can_pause(hwParams);
    LOG_INFO_IF(canPause) << "Audio device supports pausing.";
    LOG_INFO_IF(!canPause) << "Audio device does not support pausing.";

    snd_pcm_hw_params_free(hwParams);

    if ((err = snd_pcm_prepare(captureHandle)) < 0) {
        LOG_ERROR << "cannot prepare audio interface for use " << snd_strerror(err);
        return;
    }

    setVolume(settings.soundSubDevice.c_str());
}

void AlsaRecorder::setVolume(const char *subdevice) {
    int err;
    long vmin;
    long vmax;
    snd_mixer_t *mixer;
    snd_mixer_selem_id_t *selemid;

    int mode = 0;
    if ((err = snd_mixer_open(&mixer, mode)) < 0) {
        LOG_ERROR << "unable to open mixer " << snd_strerror(err);
        return;
    }
    if ((err = snd_mixer_attach(mixer, settings.soundDeviceVol.c_str())) < 0) {
        LOG_ERROR << "unable to attach card to mixer " << snd_strerror(err);
        return;
    }
    if ((err = snd_mixer_selem_register(mixer, NULL, NULL)) < 0) {
        LOG_ERROR << "unable to register mixer " << snd_strerror(err);
        return;
    }
    if ((err = snd_mixer_load(mixer)) < 0) {
        LOG_ERROR << "unable to load mixer " << snd_strerror(err);
        return;
    }
    snd_mixer_selem_id_malloc(&selemid);
    if (selemid == NULL) {
        LOG_ERROR << "unable to allocate selemid.";
        return;
    }
    snd_mixer_selem_id_set_index(selemid, 0);
    snd_mixer_selem_id_set_name(selemid, subdevice);
    snd_mixer_elem_t *elem = snd_mixer_find_selem(mixer, selemid);
    if (elem == NULL) {
        LOG_ERROR << "unable to find selem.";
        return;
    }
    if ((err = snd_mixer_selem_get_capture_volume_range(elem, &vmin, &vmax)) < 0) {
        LOG_ERROR << "unable to get capture volume range " << snd_strerror(err);
        return;
    }
    if ((err = snd_mixer_selem_set_capture_volume_all(elem,
               (vmax * settings.volume) / 100)) < 0) {
        LOG_ERROR << "unable to set capture volume " << snd_strerror(err);
        return;
    }
    snd_mixer_selem_id_free(selemid);
    if ((err = snd_mixer_close(mixer)) < 0) {
        LOG_ERROR << "unable to close mixer " << snd_strerror(err);
        return;
    }
}

int AlsaRecorder::xrunRecovery(snd_pcm_t *handle, int err) {
    LOG_ERROR << "stream recovery";

    if (err == -EPIPE) {
        /* underrun */
        err = snd_pcm_prepare(handle);
        if (err < 0) {
            LOG_ERROR << "Can't recover from underrun, prepare failed: " << snd_strerror(
                          err);
        }
        return 0;
    }
    return err;
}

void AlsaRecorder::destroyAlsa() {
    if (captureHandle == nullptr) {
        return;
    }

    LOG_DEBUG << "ALSA-RX closed.";

    int err;
    CATCH_ALSA(snd_pcm_drop(captureHandle));
    CATCH_ALSA(snd_pcm_close(captureHandle));

    captureHandle = nullptr;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
