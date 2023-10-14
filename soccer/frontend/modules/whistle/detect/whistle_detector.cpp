/**
 * @author Felix Weiglhofer
 */
#include "whistle_detector.h"

#include <framework/logger/logger.h>
#include <framework/common/platform.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

static const std::string logDir{"/home/nao/whistle/"};

WhistleDetector::WhistleDetector(AudioStream* audioProvider,
                                 const size_t minWhistleLength, const float whistleThreshold)
    : _audioProvider(audioProvider),
      _mergeChannels(_audioProvider->getBuffer(),
                     _audioProvider->getBufferSize(),
                     _audioProvider->getChannelNum()),
      _fourierTransform(_mergeChannels.output(), _mergeChannels.outSize(),
                        _audioProvider->getSampleRate()),
      _rectSmooth(3, _fourierTransform.output(), _fourierTransform.outSize(),
                  _fourierTransform.freqSpacing()),
      _whistleClassifier(_rectSmooth.output(), _rectSmooth.outSize(),
                         whistleThreshold, minWhistleLength,
                         _mergeChannels.outSize(), _rectSmooth.freqSpacing(),
                         _rectSmooth.freqOffset()),
      _fileWriter(_audioProvider->getBuffer(), _audioProvider->getBufferSize(),
                  _audioProvider->getSampleRate(),
                  _audioProvider->getChannelNum()),
      _running(false) {}

bool WhistleDetector::process() {
    jsassert(_running);

    /* TimestampMs start = getTimestampMs(); */
    _audioProvider->fetch();
    /* TimestampMs endFetch = getTimestampMs(); */

    /* _fileWriter.write(); */
    _mergeChannels.execute();
    _fourierTransform.execute();
    _rectSmooth.execute();
    _whistleClassifier.execute();

    /* TimestampMs end =  getTimestampMs(); */

    /* LOG_DEBUG_EVERY_N(40) */
    /*     << "Whistle timings: fetch audio = " << endFetch - start */
    /*     << "ms, whistle detection = " << end - endFetch << "ms"; */

    return _whistleClassifier.whistleDetected();
}

void WhistleDetector::onlyRecord() {
    jsassert(_running);
    _audioProvider->fetch();
    _fileWriter.write();
}

void WhistleDetector::start() {
    jsassert(!_running);
    _audioProvider->start();

    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    mkdir(logDir.c_str(), 0755);
    std::string fname{logDir};
    fname += std::to_string(tv.tv_sec) + ".wav";
    _fileWriter.open(fname);

    _running = true;
}

void WhistleDetector::stop() {
    jsassert(_running);
    _fileWriter.close();
    _audioProvider->stop();
    _whistleClassifier.reset();
    _running = false;
}
