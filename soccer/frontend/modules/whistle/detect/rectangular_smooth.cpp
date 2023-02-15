/**
 * @author Felix Weiglhofer
 */
#include "rectangular_smooth.h"

#include <framework/util/assert.h>

#include <numeric>


RectangularSmooth::RectangularSmooth(unsigned int smoothWidth,
                                     const float *const input,
                                     size_t inSize, float freqSpacing)
    : _input(input)
    , _inSize(inSize)
    , _output(nullptr)
    , _outSize(0)
    , _smoothWidth(smoothWidth)
    , _freqSpacing(freqSpacing) {

    jsassert(_input != nullptr);
    jsassert(_smoothWidth % 2 == 1);
    jsassert(_inSize >= _smoothWidth);

    unsigned int cutOff = _smoothWidth - 1;

    _outSize = _inSize - cutOff;
    _output  = new float[_outSize];

    _freqOffset = (cutOff / 2) * _freqSpacing;
}

RectangularSmooth::~RectangularSmooth() {
    delete[] _output;
}

void RectangularSmooth::execute() {
    const float *rectStart = _input;
    for (size_t i = 0; i < _outSize; i++) {
        const float *rectEnd = rectStart + _smoothWidth;
        _output[i]  = std::accumulate(rectStart, rectEnd, 0);
        _output[i] /= _smoothWidth;
        rectStart++;
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:
