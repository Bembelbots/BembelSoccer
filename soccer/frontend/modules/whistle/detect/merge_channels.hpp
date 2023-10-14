/**
 * @author Felix Weiglhofer
 */
#pragma once

#include <framework/util/assert.h>

#include <cstdint>
#include <cstddef>
#include <numeric>


/**
 * Merge audio data from multiple channels.
 */
template<void *(*customMalloc)(size_t), void (*customFree)(void *)>
class MergeChannels {

public:
    explicit MergeChannels(const int16_t *const input, size_t inSize,
                           unsigned int channelNum)
        : _input(input)
        , _inSize(inSize)
        , _outSize(inSize / channelNum)
        , _output(reinterpret_cast<float*>(customMalloc(_outSize * sizeof(float))))
        , _channelNum(channelNum) {
        jsassert(_inSize % _channelNum == 0);
    }

    ~MergeChannels() {
        customFree(_output);
    }

    void execute() {
        const int16_t *start = _input;
        for (size_t i = 0; i < _outSize; i++) {
            const int16_t *end = start + _channelNum;
            _output[i] = std::accumulate(start, end, 0);
            _output[i] /= _channelNum;
            start = end;
            /* _output[i] = 0; */
            /* for (unsigned int j = 0; j < _channelNum; j++) { */
            /*     _output[i] += _input[i * _channelNum + j]; */
            /* } */
        }
    }

    float *output() {
        return _output;
    }
    size_t outSize() const {
        return _outSize;
    }

private:

    const int16_t *_input;
    size_t _inSize;

    size_t _outSize;
    float *_output;

    unsigned int _channelNum;

};

// vim: set ts=4 sw=4 sts=4 expandtab:
