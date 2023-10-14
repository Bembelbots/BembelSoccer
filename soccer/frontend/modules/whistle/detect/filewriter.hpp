#pragma once

#include <cstdio>

#include "wavheader.h"

/**
 *  writes raw audio stream to file, with WAV/PCM header
 */

class FileWriter {
public:
    explicit FileWriter(const int16_t *const input, size_t inSize,
                           uint32_t sampleRate, uint8_t channelNum):
        of(nullptr),
        buf(input),
        sampleSize(inSize),
        chanNum(channelNum),
        bytesPerSample(sizeof(*buf)) {
            wavheader.channels = chanNum;
            wavheader.bits_per_sample = sizeof(*buf)*8;
            wavheader.bytes_by_capture = chanNum * wavheader.bits_per_sample/8;
            wavheader.frequency = sampleRate;
            wavheader.bytes_per_second = sampleRate * wavheader.bytes_by_capture;
    }

    ~FileWriter() {
        close();
    }

    void open(const std::string &fname) {
        if ((of = fopen(fname.c_str(), "w")))
            fseek(of, sizeof(wavheader), SEEK_SET);
    }

    void close() {
        if (of) {
            bytesWritten *= bytesPerSample;
            wavheader.totallength = sizeof(wavheader) + bytesWritten - 8;
            wavheader.bytes_in_data = bytesWritten;
            fseek(of, 0, SEEK_SET);
            fwrite(reinterpret_cast<char*>(&wavheader), 1, sizeof(wavheader), of);
            fclose(of);
        }
        of = nullptr;
    }

    void write() {
        if (of)
            bytesWritten += fwrite(buf, bytesPerSample, sampleSize*chanNum, of);
    }

private:
    FILE *of;
    const int16_t *buf;
    size_t sampleSize;
    uint8_t chanNum;
    uint8_t bytesPerSample;
    WAV::header wavheader;
    size_t bytesWritten{0};
};


// vim: set ts=4 sw=4 sts=4 expandtab:
