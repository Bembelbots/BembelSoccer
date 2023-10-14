/**
 * @author Felix Weiglhofer
 */
#pragma once

#include <fftw3.h>

/**
 * Performs a fourier transform on the input data.
 */
class FourierTransform {

public:
    /**
     * @param input: Input signal.
     * @param inSize: Size of input.
     * @param sampleRate: Samplerate of input signal.
     */
    explicit FourierTransform(float *const input, size_t inSize, float sampleRate);
    ~FourierTransform();

    FourierTransform(const FourierTransform&) = delete;
    FourierTransform& operator=(const FourierTransform&) = delete;

    void execute();

    const float *output() const {
        return _output;
    }
    size_t outSize() const {
        return _outSize;
    }

    float freqSpacing() const {
        return _freqSpacing;
    }

private:
    static inline float length(const fftwf_complex &);

    float *_input;
    size_t _inSize;

    size_t _outSize;
    fftwf_complex *_fftwOut;
    float *_output;

    fftwf_plan _plan;

    float _freqSpacing;

};

// vim: set ts=4 sw=4 sts=4 expandtab:
