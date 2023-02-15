#include "body.h"

static inline float deadband_filter(float min, float value, float max){
    if(value<0) {
        value-=min;
        if(value>0) {
            value=0;
        }
    }
    if(value>0) {
        value-=max;
        if(value<0) {
            value=0;
        }
    }
    return value;
}

void Body::proceed() {
    pitch_error = PitchError {
        .pitch = deadband_filter(body_pitch_deadband_min, body_pitch, body_pitch_deadband_max),
        .gyro = deadband_filter(-gyro_deadband, gyro_pitch, gyro_deadband)
    };
}
