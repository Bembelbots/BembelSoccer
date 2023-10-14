#pragma once
#include "bodycontrol/internals/submodule.h"

#include <iostream>
#include <cmath>
#include <bitset>


#define r_eye(i, r, g, b) EYE(faceLedRedRight0DegActuator, i, r, g, b)
#define l_eye(i, r, g, b) EYE(faceLedRedLeft0DegActuator, i, r, g, b)

#define EYE(start, i , r, g, b)\
    actuators[start+i] = r;\
    actuators[start+i+8] = g;\
    actuators[start+i+16] = b;


class LedDisco : public SubModule {
public:
    LedDisco():
        eye_switch(),
        cycle(0),
        v1(0),
        v2(0),
        p1(0),
        p2(0),
        kill_me(false) {}

public:
    SubModuleReturnValue step(BodyBlackboard * bb) override {
        return discoLights(bb->actuators.get().data());
    }


    SubModuleReturnValue discoLights(float *actuators) {
        if (kill_me){
            for (int i = 0; i < 8; i++){
                l_eye(i,0,0,0);
                r_eye(i,0,0,0);
            }
            return SubModuleReturnValue::DEACTIVATE_ME;
        }

        int maxcycle = 250;
        cycle += 1; cycle %= maxcycle;
        if (cycle % 1 == 0){
            v1 += 1; v1 %= 3;
            v2 += 1; v2 %= 7;
            if (v1 == 0){
                p1 += 1; p1 %= 8;
                eye_switch[p1] = not eye_switch[p1];
            }
            if (v2 == 0){
                p2 += 1; p2 %= 8;
                eye_switch[p2] = not eye_switch[p2];
            }
        }
        float phi = float(cycle)/float(maxcycle)*2.0f*M_PI_F;
        for (int i = 0; i < 8; i++){
            if (eye_switch[i]){
                r_eye(i, cos(phi), cos(phi+2.f/3.f*M_PI_F), cos(phi+ 4.f/3.f*M_PI_F));
                l_eye(i, cos(phi), cos(phi+2.f/3.f*M_PI_F), cos(phi+ 4.f/3.f*M_PI_F));
            } else {
                l_eye(i,0,0,0);
                r_eye(i,0,0,0);
            }
        }
        for (int i = faceLedRedLeft0DegActuator; i <=  rFootLedBlueActuator; i++){
            if (actuators[i] < 0) actuators[i] = 0;
            if (actuators[i] > 1) actuators[i] = 1;
        }
        return SubModuleReturnValue::MOTION_STABLE;
        
    }
    

private:
    std::bitset<8> eye_switch;
    int cycle;
    int v1;
    int v2;
    int p1;
    int p2;
    bool kill_me;
};
