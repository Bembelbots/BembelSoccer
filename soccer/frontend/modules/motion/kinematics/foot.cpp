#include "foot.h"
#include <cmath>
#include <initializer_list>

float calcSupportFoot(Foot fsr) {
    static constexpr float max_pressure = 5.0f;
    static const Foot weights{foot_s{0.8f, 0.3f, 0.8f, 0.3f}, foot_s{-0.3f, -0.8f, -0.3f, -0.8f}}, fsr_max_values;
    float total = 0;
    float weighted = 0;
    for (foot_s* f : {&fsr.left, &fsr.right}) {
        for (float* v : {&f->fl, &f->fr, &f->rl, &f->rr}) {
            *v = std::min(max_pressure, *v);
            float* m = (float*)&fsr_max_values + ((float*)v - (float*)&fsr);
            float* w = (float*)&weights + ((float*)v - (float*)&fsr);
            *m = std::max(*v, *m);
            if(*m == 0){
                continue;
            }
            *v /= *m;
            total += *v;
            weighted += *w * *v;
        }
    }

    if (total == 0) {
        return 0;
    }

    return weighted / total;
}