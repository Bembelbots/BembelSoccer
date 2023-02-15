/**
 * @author Felix Weiglhofer
 */
#pragma once

#include "alsarecorder.h"
#include "filewriter.hpp"
#include "fourier_transform.h"
#include "merge_channels.hpp"
#include "rectangular_smooth.h"
#include "whistle_classifier.h"

#include <fftw3.h>

class WhistleDetector {

   public:
    WhistleDetector(AudioStream*, const size_t minWhistleLength,
                    const float whistleThreshold);

    bool process();

    void onlyRecord();

    /**
     * Start whistle detection. Must be called before calling process.
     */
    void start();

    void stop();

    bool isRunning() const {
        return _running;
    }

    bool hasCandidate() const {
        return _whistleClassifier.hasCandidate();
    }

    float currPeak() const {
        return _whistleClassifier.currPeak();
    }

   private:
    AudioStream* _audioProvider;

    // Whistle detection pipeline
    MergeChannels<fftwf_malloc, fftwf_free> _mergeChannels;
    FourierTransform _fourierTransform;
    RectangularSmooth _rectSmooth;
    WhistleClassifier _whistleClassifier;
    FileWriter _fileWriter;

    bool _running;
};
