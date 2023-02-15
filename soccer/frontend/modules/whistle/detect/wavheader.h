/**
 * @author Felix Weiglhofer
 */
#pragma once

namespace WAV {
    struct header {
        char    id[4]{'R','I','F','F'};                         // should always contain "RIFF"
        int32_t totallength;                                    // total file length minus 8
        char    wavefmt[8]{'W','A','V','E','f','m','t',' '};    // should be "WAVEfmt "
        int32_t format{16};                                     // 16 for PCM format
        int16_t pcm{1};                                         // 1 for PCM format
        int16_t channels{2};                                    // channels
        int32_t frequency;                                      // sampling frequency
        int32_t bytes_per_second;                               // frequency * bytes_per_capture
        int16_t bytes_by_capture;                               // bits_per_sample / 8 * channels
        int16_t bits_per_sample{16}; 
        char    data[4]{'d','a','t','a'};                       // should always contain "data"
        int32_t bytes_in_data;                                  // total number of data bytes following
    } __attribute__((__packed__));
}

// vim: set ts=4 sw=4 sts=4 expandtab:
