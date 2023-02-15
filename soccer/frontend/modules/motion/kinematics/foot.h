#pragma once

struct foot_s {
    float fl=0;
    float fr=0;
    float rl=0;
    float rr=0;
    float avg=0;

    foot_s() {}
    foot_s(float fl, float fr, float rl, float rr) : fl(fl), fr(fr), rl(rl), rr(rr) {}
};

struct Foot {
    foot_s left;
    foot_s right;
    Foot() {}
    Foot(const foot_s& left, const foot_s& right) : left(left), right(right) {}
};

float calcSupportFoot(Foot fsr);