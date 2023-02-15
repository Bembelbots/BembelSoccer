#pragma  once
#include <cmath>
#include <initializer_list>
#include "htwk/utils/fsr.h"

float parabolicReturn(float f) {
    if (f < 0.25f) {
        return 8.f * f * f;
    } else if (f < 0.75f) {
        float v = f - 0.5f;
        return 1.f - 8.f * v * v;
    } else {
        float v = 1.f - f;
        return 8.f * v * v;
    }
}

float get_spline_value(float t, float* spline, int size) {
    for (int j = 1; j < size; j++) {
        for (int i = 0; i < size - j; i++) {
            spline[j * size + i] =
                    spline[(j - 1) * size + i] * (1 - t) + spline[(j - 1) * size + i + 1] * t;
        }
    }
    return (spline[(size - 1) * size]);
}

float linearParameterChange(float in, float target, float delta) {
    if (in < target) {
        return std::min(in + delta, target);
    } else if (in > target) {
        return std::max(in - delta, target);
    }
    return target;
}

// positive is left foot
float calcSupportFoot(FSR fsr_current, FSR &fsr_max) {
    static constexpr float max_pressure = 5.0f;
    static const FSR weights{.left = {0.8f, 0.3f, 0.8f, 0.3f}, .right = {-0.3f, -0.8f, -0.3f, -0.8f}};
    float total = 0;
    float weighted = 0;
    for (Foot* f : {&fsr_current.left, &fsr_current.right}) {
        for (float* v : {&f->fl, &f->fr, &f->rl, &f->rr}) {
            *v = std::min(max_pressure, *v);
            float* m = (float*)&fsr_max + ((float*)v - (float*)&fsr_current);
            float* w = (float*)&weights + ((float*)v - (float*)&fsr_current);
            *m = std::max(*v, *m);
            if (*m > 0.f) {
                *v /= *m;
                total += *v;
                weighted += *w * *v;
            }
        }
    }
    if (total == 0) {
        return 0;
    }
    return weighted / total;
}
