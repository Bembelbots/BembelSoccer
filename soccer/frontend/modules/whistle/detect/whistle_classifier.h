/**
 * @author Felix Weiglhofer
 */
#pragma once

#include <cstddef>

/**
 * Classifies audio data as a whistle or not.
 */
class WhistleClassifier {

public:

    /**
     * @param input: Audio data in frequency domain.
     * @param inSize: Size of input.
     * @param whistleThreshold: Classify input as a whistle if the peak in
     * input is higher than whistleThreshold (Unit: Hz)
     * @param minWhistleLength: Only accept a signal as a whistle if its at
     * least minWhistleLength samples long.
     * @param bufferLength: Number of audio samples that is processed with
     * every processing step.
     * @param freqSpacing: Spacing between entries in input.
     * @param freqOffset: Frequency offset of the first entry of input.
     */
    explicit WhistleClassifier(
        const float *const input,
        size_t inSize,
        float whistleThreshold,
        unsigned int minWhistleLength,
        float bufferLength,
        float freqSpacing,
        float freqOffset);

    void execute();

    bool whistleDetected() const {
        return _whistleDetected;
    }

    bool hasCandidate() const {
        return _matchLength > 0;
    }

    float currPeak() const {
        return _currPeak;
    }

    void reset();

private:
    const float *_input;
    size_t _inSize;

    float _bufferLength;
    float _freqSpacing;
    float _freqOffset;

    size_t _matchLength;
    size_t _minMatchLength;
    float _currPeak;

    float _whistleThreshold;

    bool _whistleDetected;

    float findPeak();

};

// vim: set ts=4 sw=4 sts=4 expandtab:
