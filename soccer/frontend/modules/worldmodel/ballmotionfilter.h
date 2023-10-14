/** @author Bembelbots Frankfurt , Jonathan Brast <brast> in the domain of cs.uni-frankfurt.de
 */

#pragma once

#include <framework/util/clock.h>
#include <framework/math/old.h>

#include <math.h>

/*! One type for all the cool Information
 * @param position The last position as (x position , y position , timestamp , confidence).
 * @param velocity The calculated velocity as (x velocity , y velocity , timestamp , confidence).
 * @param baselineCrossings The crossing points for the baselines (x(y=0) , y(x=0) , timestamp , confidence).
 * @param baselineTimestamps The timestamp baselines crossings (t(y=0) , t(x=0) , timestamp , confidence).
 */
typedef struct {
    Measurement2D position;
    Measurement2D velocity;
    Measurement2D baselineCrossings;
    Measurement2D baselineTimestamps;
} BallInfo;

/*! Does the math: a linear regression by accumulation of position data*/
class BallRegression {
public:
    BallRegression();
    void add(float px, float py, float pt);
    void calculate();
    void reset();
    // results
    float yIntersection;
    float yInclination;
    float xIntersection;
    float xInclination;
    float yStart;
    float yVelocity;
    float xStart;
    float xVelocity;
    float timeOffset;
    float baselineTime;
    float mse_time;
    float mse_pos;
    float confidence;
    // data set size
    int n;
    // Status flags
    bool frictionUsed;
    bool approaching;
    bool hittingBaseline;
    bool ready;
    // Accumulators
    float At;
    float Att;
    float Ay;
    float Ayy;
    float Ax;
    float Ayx;
    float Ayt;
    float Axt;
    float Axx;

};

/*! Wraps around BallRegression, decides when to start a new regression. */
class BallHypothesis {
public:
    BallHypothesis();
    void add(float rcsX, float rcsY, float time);
    void add(const Measurement2D &pos);
    void reset();

    // Memorisation
    float lastX;
    float lastY;
    float lastTime;

    // counting for smoothing
    int smallVelocityCount;
    int falseDirectionCount;

    // Status variables
    bool isStanding;
    int ticksStanding;
    int ticksMoving;

    // tuning variables
    float smallVelocityThreshold;
    float longTimeThreshold;
    float wrongDirectionAngleThreshold;
    int   minimalSizeToConsiderDirection;
    int   nrOfVelocityViolationsBeforeReset;
    int   nrOfDirectionViolationsBeforeReset;

    // sanity check variables
    float ignoreableDistance;
    float ignoreableVelocity;
    float sanity;

    BallRegression moving;
};



/*!
 * Predicts the movements of a ball based on linear regression.
 * This is what you should interface with (readable names, interpretation of data, ...)
 */
class JonathansBallMotionFilter {
public:
    float lastDataTime;
    float confidence;
    TimestampMs lastTimestamp;
    BallHypothesis hypothesis;

    void debugOutput(); // cppcheck-suppress functionStatic
    // Status
    bool  isStanding() const;
    bool  isMoving() const;
    bool  hitsBaseline();
    bool  approaching();
    float getConfidence();

    // Prediction
    //! @return estimation of distance where the ball crosses the RCS baseline (in m)
    float hitsBaselineWhere();
    //! @return estimation of time when the ball crosses the RBS baseline (timestamp (in ms) (float))
    float hitsBaselineWhen();
    //! Tries to predict a future or past ball position (in m)
    float xPosAtTime(float timestamp);
    //! Tries to predict a future or past ball position (in m)
    float yPosAtTime(float timestamp);

    // Velocity
    //! @return Velocity in x-direction (in m/s)<
    float xVelocity();
    //! @restur Velocity in y-direction (in m/s)
    float yVelocity();
    //! @resturn Velocity (in m/s)
    float velocity();

    // Data Input
    /*! Adds cartesian coordinate points to filter from rcs
     * @param rcsX X-coordinate (left/right) (in m)
     * @param rcsY Y-coordinate (front/back) (in m)
     * @param timestamp timestamp (in ms)
     */
    void  addPointXY(float rcsX, float rcsY, TimestampMs timestamp);
    /*! Adds cartesian coordinate points to filter from rcs
     * @param rcsX X-coordinate (left/right) (in m)
     * @param rcsY Y-coordinate (front/back) (in m)
     * @param timestamp timestamp (in ms)
     */
    void  addPointXY(float rcsX, float rcsY, float timestamp);
    /*! Adds polar coordinate points to filter from rcs
     * @param rcsR Radius (distance) (in m)
     * @param rcsPhi Radiant (angle) circumfence lenght of an unit circle
     * @param timestamp timestamp (in ms)
     */
    void  addPointPol(float rcsR, float rcsPhi, int timestamp);
    /*! Adds polar coordinate points to filter from rcs
     * @param rcsR Radius (distance) (in m)
     * @param rcsPhi Radiant (angle) circumfence lenght of an unit circle
     * @param timestamp timestamp (in ms)
     */
    void  addPointPol(float rcsR, float rcsPhi, float timestamp);
    //! Assures all values include most recent data.
    void  calculate();
    //! generates a confidence value after performing some sanity checks
    float calculateConfidence() const;




    // Tuning
    //! Before this number of values have been added the direction is considered too unprecise
    void tuneMinNrOfValuesToConsiderDirection(int nrOfValues);
    //! If the filter waited longer than this time it resets (in s)
    void tuneMaxTimeBetweenData(float maxTime);
    //! If the velocity of the last two points is smaller than this the ball is considered standing (in m/s)
    void tuneVelocityConsideredStanding(float velocity);
    //! If the ball stood for this number of frames the filter will reset
    void tuneMaxNrOfVelocityViolations(int maxNr);
    //! If the velocity of the last two points points in a direction more than this angle the ball is considered not to be following the regression anymore (in °)
    //! This is only relevant if there are a sufficient number of values
    void tuneAngleConsideredWrong(float degrees);
    //! If the ball had a false direction for this number of frames the filter will reset
    void tuneMaxNrOfDirectionViolations(int maxNr);


    void updateParameters(int minNrOfValues, int maxVelocityViolations,
                          int maxAngleViolations, float velocityThreshold, float angleThreshold,
                          float maxTimeInterval);




    /*! Function for BallMotionFilter compatibility
     */
    void insertPosMeasurement(const Measurement2D &pos);
    /*! Function for BallMotionFilter compatibility
     */
    Measurement2D getFilteredSpeed();



};

// vim: set ts=4 sw=4 sts=4 expandtab:
