/*
 * This file contains the classes BallMotion, BallRegression and BallHypothesis.
 * BallMotion is how you should interface with the everything here.
 * BallHypotheis resets the Regression under certain circumstances.
 * BallRegession does the accounting and the math.
 *
 */


//#define JONATHAN_DEBUG
#include "ballmotionfilter.h"
#include <stdio.h>
#include <math.h>
#include <boost/math/special_functions/fpclassify.hpp>

static const float EPSILON = 1e-6;    // Epsilon for float comparison


using namespace std;

void JonathansBallMotionFilter::debugOutput() {
    //cout << "BLT:" << hypothesis.moving.baselineTime/1000.0f << " v_x:" << hypothesis.moving.xVelocity*1000.0f ;
    //cout << " _X_:" << hypothesis.lastX << " GMT:" << hypothesis.lastTime/1000.0f ;
    //cout << " eta:" << hypothesis.moving.baselineTime/1000.0f - (float)hypothesis.lastTime/1000.0f ;
    //cout << " d_t:" << hypothesis.moving.timeOffset/1000.0f;
    //cout << endl;
}







//================================//
// BallMotionFilter compatibility //
//================================//
void JonathansBallMotionFilter::insertPosMeasurement(const Measurement2D
        &pos) {
    lastDataTime = pos.timestamp;
    hypothesis.add(pos);
    //addPointXY(pos.x, pos.y, int(pos.timestamp));
}

Measurement2D JonathansBallMotionFilter::getFilteredSpeed() {
    return Measurement2D(xVelocity(), yVelocity(), confidence, lastTimestamp);
}

//=============//
// ball status //
//=============//

bool JonathansBallMotionFilter::isStanding() const {
    return hypothesis.isStanding;
}
bool JonathansBallMotionFilter::isMoving() const {
    return !hypothesis.isStanding;
}
bool JonathansBallMotionFilter::hitsBaseline() {
    if (isStanding()) {
        return false;
    }
    calculate();
    return hypothesis.moving.hittingBaseline;
}
bool JonathansBallMotionFilter::approaching() {
    if (isStanding()) {
        return false;
    }
    calculate();
    return hypothesis.moving.approaching;
}
float JonathansBallMotionFilter::getConfidence() {
    calculate();
    return confidence;
}

//=====================//
// baseline parameters //
//=====================//

float JonathansBallMotionFilter::hitsBaselineWhere() {
    calculate();
    return hypothesis.moving.yIntersection;
}
float JonathansBallMotionFilter::hitsBaselineWhen() {
    calculate();
    //return hypothesis.moving.yIntersection;
    return hypothesis.moving.baselineTime;
}

//==========//
// position //
//==========//

float JonathansBallMotionFilter::xPosAtTime(float timestamp) {
    return hypothesis.moving.yIntersection + hypothesis.moving.yInclination *
           yPosAtTime(timestamp);
}
float JonathansBallMotionFilter::yPosAtTime(float timestamp) {
    calculate();
    float t = timestamp - hypothesis.moving.timeOffset;
    return hypothesis.moving.yStart + hypothesis.moving.yVelocity * t;
}

//==========//
// velocity //
//==========//


float JonathansBallMotionFilter::yVelocity() {
    calculate();
    float v = hypothesis.moving.yVelocity * 1000;
    return v;
}
float JonathansBallMotionFilter::xVelocity() {
    calculate();
    float v = hypothesis.moving.xVelocity * 1000;
    return v;
}
float JonathansBallMotionFilter::velocity() {
    calculate();
    float yVel = hypothesis.moving.yVelocity;
    float xVel = hypothesis.moving.xVelocity;
    return sqrt(xVel * xVel + yVel * yVel) * 1000;
}





//=======================//
// input and calculation //
//=======================/

void JonathansBallMotionFilter::addPointXY(float rcsX, float rcsY,
        TimestampMs timestamp) {
    lastTimestamp = timestamp;
    addPointXY(rcsX, rcsY, float(timestamp));
}
void JonathansBallMotionFilter::addPointXY(float rcsX, float rcsY,
        float timestamp) {
    lastDataTime = timestamp;
    if (fabs(float(lastTimestamp) - lastDataTime) < EPSILON) {
        lastTimestamp = int(lastDataTime);
    }
    hypothesis.add(rcsX, rcsY, timestamp);
}
void JonathansBallMotionFilter::addPointPol(float rcsR, float rcsPhi,
        int timestamp) {
    lastTimestamp = timestamp;
    addPointPol(rcsR * sin(rcsPhi), rcsR * cos(rcsPhi), float(timestamp));
}
void JonathansBallMotionFilter::addPointPol(float rcsR, float rcsPhi,
        float timestamp) {
    lastDataTime = timestamp;
    if (fabs(float(lastTimestamp) - lastDataTime) < EPSILON) {
        lastTimestamp = int(lastDataTime);
    }
    hypothesis.add(rcsR * sin(rcsPhi), rcsR * cos(rcsPhi), timestamp);
}
void JonathansBallMotionFilter::calculate() {
    if (hypothesis.moving.ready) {
        return;
    }
    hypothesis.moving.calculate();
    if (!boost::math::isnormal(hypothesis.moving.yIntersection)) {
        hypothesis.moving.yIntersection=0.0f;
    }
    if (!boost::math::isnormal(hypothesis.moving.yVelocity)) {
        hypothesis.moving.yVelocity=0.0f;
    }
    if (!boost::math::isnormal(hypothesis.moving.xVelocity)) {
        hypothesis.moving.xVelocity=0.0f;
    }
    if (!boost::math::isnormal(hypothesis.moving.yInclination)) {
        hypothesis.moving.yInclination=0.0f;
    }
    if (!boost::math::isnormal(hypothesis.moving.baselineTime)) {
        hypothesis.moving.baselineTime=0.0f;
    }
    confidence=calculateConfidence();
    if (hypothesis.isStanding) {
        confidence = 0.0f;
    }
    if (!hypothesis.moving.ready) {
        confidence = 0.0f;
    }
}


//========//
// tuning //
//========//
void JonathansBallMotionFilter::tuneMinNrOfValuesToConsiderDirection(
    int nrOfValues) {
    hypothesis.minimalSizeToConsiderDirection = nrOfValues;
}
void JonathansBallMotionFilter::tuneMaxTimeBetweenData(float maxTime) {
    hypothesis.longTimeThreshold = maxTime;
}

void JonathansBallMotionFilter::tuneVelocityConsideredStanding(
    float velocity) {
    hypothesis.smallVelocityThreshold = velocity;
}
void JonathansBallMotionFilter::tuneMaxNrOfVelocityViolations(int maxNr) {
    hypothesis.nrOfVelocityViolationsBeforeReset = maxNr;
}
void JonathansBallMotionFilter::tuneAngleConsideredWrong(float degrees) {
    hypothesis.wrongDirectionAngleThreshold = 1 - (degrees / 90.0f);
}
void JonathansBallMotionFilter::tuneMaxNrOfDirectionViolations(int maxNr) {
    hypothesis.nrOfDirectionViolationsBeforeReset = maxNr;
}

void JonathansBallMotionFilter::updateParameters(int minNrOfValues,
        int maxVelocityViolations, int maxAngleViolations, float velocityThreshold,
        float angleThreshold, float maxTimeInterval) {
    if (minNrOfValues != hypothesis.minimalSizeToConsiderDirection) {
        hypothesis.minimalSizeToConsiderDirection = minNrOfValues;
        hypothesis.reset();
    }
    if (maxVelocityViolations != hypothesis.nrOfVelocityViolationsBeforeReset) {
        hypothesis.nrOfVelocityViolationsBeforeReset = maxVelocityViolations;
        hypothesis.reset();
    }
    if (maxAngleViolations != hypothesis.nrOfDirectionViolationsBeforeReset) {
        hypothesis.nrOfDirectionViolationsBeforeReset = maxAngleViolations;
        hypothesis.reset();
    }
    if (fabs(velocityThreshold - hypothesis.smallVelocityThreshold) >
            EPSILON) {
        hypothesis.smallVelocityThreshold = velocityThreshold;
        hypothesis.reset();
    }
    if (fabs(angleThreshold - hypothesis.wrongDirectionAngleThreshold) >
            EPSILON) {
        hypothesis.wrongDirectionAngleThreshold = angleThreshold;
        hypothesis.reset();
    }
    if (fabs(maxTimeInterval - hypothesis.longTimeThreshold) > EPSILON) {
        hypothesis.longTimeThreshold = maxTimeInterval;
        hypothesis.reset();
    }
}


////
//// Interface ends here
//// Math starts here
////


BallRegression::BallRegression() :
    yIntersection(0.0f),
    yInclination(0.0f),
    xIntersection(0.0f),
    xInclination(0.0f),
    yStart(0.0f),
    yVelocity(0.0f),
    xStart(0.0f),
    xVelocity(0.0f),
    timeOffset(0.0f),
    baselineTime(0.0f),
    mse_time(0.0f),
    mse_pos(0.0f),
    confidence(0.0f),
    n(0),
    frictionUsed(false),
    approaching(false),
    hittingBaseline(false),
    ready(false),
    At(0.0f),
    Att(0.0f),
    Ay(0.0f),
    Ayy(0.0f),
    Ax(0.0f),
    Ayx(0.0f),
    Ayt(0.0f),
    Axt(0.0f),
    Axx(0.0f) {
    reset();
}


void BallRegression::reset() {
    yIntersection = 0.0f;
    yInclination = 0.0f;
    xIntersection = 0.0f;
    xInclination = 0.0f;
    yStart = 0.0f;
    yVelocity = 0.0f;
    xStart = 0.0f;
    xVelocity = 0.0f;
    timeOffset = 0.0f;
    baselineTime = 0.0f;
    mse_time = 0.0f;
    mse_pos = 0.0f;
    confidence = 0.0f;
    n = 0;
    frictionUsed = false;
    approaching = false;
    hittingBaseline = false;
    ready = false;
    At = 0.0f;
    Att = 0.0f;
    Ay = 0.0f;
    Ayy = 0.0f;
    Ax = 0.0f;
    Ayx = 0.0f;
    Ayt = 0.0f;
    Axt = 0.0f;
    Axx = 0.0f;
}

void BallRegression::add(float px, float py, float pt) {
    if (n == 0) {
        timeOffset = pt;
    }
    pt -= timeOffset;
    n++;
    ready = false;
    At      += pt;
    Ay      += py;
    Ax      += px;
    Att     += pt * pt;
    Ayy     += py * py;
    Ayx     += py * px;
    Ayt     += py * pt;
    Axt     += px * pt;
    Axx     += px * px;
}

void BallRegression::calculate() {
    frictionUsed = true;
    ready = true;
    float Cyt, Ctt, Cyx, Cyy, Cxt, Cxx;
    // calc of: Covariances
    Ctt     = (Att   - At * At / n) / n;
    Cyy     = (Ayy   - Ay * Ay / n) / n;
    Cxx     = (Axx   - Ax * Ax / n) / n;
    Cyt     = (Ayt   - Ay * At / n) / n;
    Cyx     = (Ayx   - Ay * Ax / n) / n;
    Cxt     = (Axt   - Ax * At / n) / n;

    // calc of: Regression Parameters for Y Time Function Linear
    yVelocity = Cyt / Ctt;
    yStart = (Ay - yVelocity * At) / n;

    // calc of: Regression Parameters for X Time Function Linear
    xVelocity = Cxt / Ctt;
    xStart = (Ax - xVelocity * At) / n;

    // calc of: Regression Parameters for Position Function
    yInclination = Cyx / Cxx;
    yIntersection = (Ay - yInclination * Ax) / n; // Point where x=0

    xInclination = Cyx / Cyy;
    xIntersection = (Ax - xInclination * Ay) / n; // Point where y=0
    /*  y
     *  |
     *  |     /
     *  |    / dy/dx=yInclination
     *  |   /
     *  |  /
     *  +-o----------x
     *  |/
     *  o
     *  |
     *  |
     *
     *
     *
     */

    frictionUsed = false;
    if (xVelocity < 0) {
        //cout << "\x1b[0;31;40m < \x1b[0m";
        approaching = true;
        hittingBaseline = true;
        baselineTime = -xStart / xVelocity;
        baselineTime += timeOffset;
    } else {
        //cout << "\x1b[0;35;40m > \x1b[0m";
        approaching = false;
        hittingBaseline = false;
        baselineTime=0.0f;
    }
}

float JonathansBallMotionFilter::calculateConfidence() const {
    if (hypothesis.isStanding) {
        return 0.0f;
    }
    if (!hypothesis.moving.ready) {
        return 0.0f;
    }
    return hypothesis.moving.confidence;
}


BallHypothesis::BallHypothesis() :
    lastX(0.0f),
    lastY(0.0f),
    lastTime(0.0f),
    smallVelocityCount(0),
    falseDirectionCount(0),
    isStanding(true),
    ticksStanding(0),
    ticksMoving(0),
    // WARNING CHANGES TO THE FOLLOWING VARIABLES WILL NOT CHANGE ANYTHING
    // THIS IS NOW DONE IN POSE BLACKBOARD
    smallVelocityThreshold(0.0f),    // original 2.75
    longTimeThreshold(0.0f),            // original 1.0
    wrongDirectionAngleThreshold(0.0f), // 90deg original 0.5f == 45°
    minimalSizeToConsiderDirection(0),  // original 3
    nrOfVelocityViolationsBeforeReset(0),   // original 5
    nrOfDirectionViolationsBeforeReset(0), // original 3
    ignoreableDistance(0.0f),
    ignoreableVelocity(0.0f),
    sanity(1.0f) {
}

void BallHypothesis::add(const Measurement2D &pos) {
    add(pos.x, pos.y, pos.timestamp);
}

void BallHypothesis::add(float rcsX, float rcsY, float time) {
    if (time - lastTime <= 0) {     //two cams same time?
        moving.add(rcsX, rcsY, time);
        return;
    }

    bool reset = false; //!< will we add data or will we reset
    if (moving.n >= 3) {
        isStanding=false;
    }
    //calculate some information we may need
    moving.calculate();
    // velocitys in m/ms!! but I don't care because only the direction is relevant!
    float regVelX = moving.xVelocity;
    float regVelY = moving.yVelocity;
    float squareRegVel = regVelX * regVelX + regVelY * regVelY;
    float simpleVelX = (rcsX - lastX) / (time - lastTime);
    float simpleVelY = (rcsY - lastY) / (time - lastTime);
    float squareSimpleVel = simpleVelX * simpleVelX + simpleVelY * simpleVelY;
    // ANNOTATION: direction = (v_reg*v_simple)/(|v_reg|^2*|v_simple|^2)
    // This is the angle between the vectors
    // 1 = 0°, 1/2 = 45°, 0 = 90°, -1/2 = 135°, -1 = 180°
    float direction = (regVelX * simpleVelX + regVelY * simpleVelY) /
                      (squareRegVel * squareSimpleVel);

    // Distance to last point
    float distX = rcsX - lastX;
    float distY = rcsY - lastY;
    float pointDistanceSquared = distX * distX + distY * distY;


    // Memoize vars for next check
    lastX = rcsX;
    lastY = rcsY;

    // Check simple velocity
    if (squareSimpleVel * 1000.0f * 1000.0f < smallVelocityThreshold *
            smallVelocityThreshold) {
        smallVelocityCount++;
    } else {
        smallVelocityCount = 0;
    }

    // Check against max point distance
    /* BROKEN IGNORE IS NOT DEFINED
    if (pointDistanceSquared > ignoreableDistance * ignoreableDistance) {
        ignore = true;
    }
    */


    //Check movement vector direction
    if (direction < wrongDirectionAngleThreshold) {
        falseDirectionCount++;
    } else {
        falseDirectionCount = 0;
    }


    // Evaluate conditions
    if ((time - lastTime) > longTimeThreshold * 1000) { // my data is history
        reset = true;
    }
    if (smallVelocityCount >= nrOfVelocityViolationsBeforeReset) {
        reset = true;
        //cout << "\x1b[1;30;41m";
        //cout << "reset: velo";
        //cout << "\x1b[0m" ;
        //cout << endl;
    }
    if (falseDirectionCount >= nrOfDirectionViolationsBeforeReset) {
        reset = true;
        //cout << "\x1b[0;33m";
        //cout << "reset: dir" ;
        //cout << "\x1b[0m" ;
        //cout << endl;
    }

    // Add or reset
    if (reset) {
        moving.reset();
        isStanding=true;
    } else {
        moving.add(rcsX, rcsY, time);
    }

    // Finally memorize time
    lastTime = time;

}

void BallHypothesis::reset() {
    lastX = 0.0f;
    lastY = 0.0f;
    lastTime = 0.0f;
    smallVelocityCount = 0;
    falseDirectionCount = 0;
    isStanding = true;
    ticksStanding = 0;
    ticksMoving = 0;
    sanity = 0.0f;
    moving.reset();
}


// vim: set ts=4 sw=4 sts=4 expandtab:
