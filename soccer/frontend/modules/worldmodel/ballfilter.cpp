#include "ballfilter.h"
#include <framework/util/clock.h>

BallFilter::BallFilter(float smoothingFactor, float max_dist):
    smoothingFactor(smoothingFactor), max_dist(max_dist){};
   
    
Coord BallFilter::processNewBall(VisionResult ball){//adds a new Ball, and returns filtered result
    if (ball.camera == TOP_CAMERA)
        return Coord(ball.rcs_x1, ball.rcs_y1);

    Coord newBall(ball.rcs_x1, ball.rcs_y1);
    if ((newBall.dist(lastBall) < max_dist) 
            and  (getTimestampMs()-timestamp_lastBall < C::max_ball_age)){
        Coord filteredBall = newBall *smoothingFactor + (1.0f-smoothingFactor) *lastBall;
        lastBall = filteredBall;
        return filteredBall;
    }
    else{
        lastBall = newBall;
        timestamp_lastBall = ball.timestamp;
        return newBall;
    }     
}
