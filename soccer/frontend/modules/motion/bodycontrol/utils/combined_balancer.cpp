#include "combined_balancer.h"

CombinedBalancer::CombinedBalancer() {
    /* logging for calibration: */
    /* lf.open("log_balancer.dat");
    lf << "gy" << " " << "gyroPitch" << " " << "pitchDelta" << " " << "shoulderPitch" << " " << "anklePitch" << " " << "gx" << " " << "gyroRoll" << " " << "rollDelta" << " " << "shoulderRollL" << " " << "shoulderRollR" << " " << "ankleRoll" << std::endl; */
}

CombinedBalancer::~CombinedBalancer() {
    // lf.close();
}

void CombinedBalancer::proceed(const float &gx, const float &gy, std::vector<float> d) {
    /* d:
    [0] maxAdditionalShoulderPitchPos
    [1] maxAdditionalShoulderPitchAnglePos
    [2] maxAdditionalShoulderPitchNeg
    [3] maxAdditionalShoulderPitchAngleNeg
    [4] maxAdditionalShoulderRollL            (pos)
    [5] maxAdditionalShoulderRollAngleL   
    [6] maxAdditionalShoulderRollR            (neg)
    [7] maxAdditionalShoulderRollAngleR 
    
    [8] shoulderForwardFactor
    [9] shoulderBackwardFactor
    [10] shoulderSideFactor
    [11] ankleForwardFactor
    [12] ankleBackwardFactor
    [13] ankleSideFactor

    [14] useShoulderPitch
    [15] useShoulderRoll
    [16] useAnklePitch
    [17] useAnkleRoll
    */

    gyroPitch += gy;
    gyroRoll += gx;

    float pitchDelta = gyroPitch * (duration / 1000);
    float rollDelta = gyroRoll * (duration / 1000);

    if (d[14]) { // useShoulderPitch
        if (pitchDelta > 0.f) { //forward falling
            shoulderPitch += std::min(d[8] * (pitchDelta/d[1]) * d[0], d[0]);
        }
        else {//if (pitchDelta < 0.f) { //backward falling
            shoulderPitch += std::max(-d[9] * (pitchDelta/d[3]) * d[2], d[2]);
        }
    } else shoulderPitch += 0.0;

    if (d[15]) { // useShoulderRoll
        if (rollDelta > 0.f) {
            shoulderRollL += std::min(d[10] * (rollDelta/d[5]) * d[4], d[4]);
            shoulderRollR += 0.0;
        }
        else {//if (rollDelta < 0.f) {
            shoulderRollL += 0.0;
            shoulderRollR += std::max(-d[10] * (rollDelta/d[7]) * d[6], d[6]);
        } 
    } else {
        shoulderRollL += 0.0;
        shoulderRollR += 0.0;
    }

    // ankle:
    anklePitch = d[16] ? (gyroPitch > 0.f ? d[11] * pitchDelta : d[12] * pitchDelta) : 0.0f;
    ankleRoll = d[17] ? (d[13] * rollDelta) : 0.0f;

    /* logging for calibration: */
    // lf << gy << " " << gyroPitch << " " << pitchDelta << " " << shoulderPitch << " " << anklePitch << " " << gx << " " << gyroRoll << " " << rollDelta << " " << shoulderRollL << " " << shoulderRollR << " " << ankleRoll << std::endl;
}
