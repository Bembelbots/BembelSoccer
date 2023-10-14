#pragma once

struct DHParameter {
    float gamma, d, a, alpha;
    bool q;

    DHParameter(float gamma, float d, float a, float alpha, bool q=true)
        : gamma(gamma), d(d), a(a), alpha(alpha), q(q) {}
};