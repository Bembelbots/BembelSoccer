#include <representations/vision/visiondefinitions.h>
#include <representations/bembelbots/constants.h>
#include <framework/math/coord.h>

namespace C = CONSTANTS;

class BallFilter{
private:
    float smoothingFactor;
    Coord lastBall;
    int timestamp_lastBall = -C::max_ball_age;
    float max_dist;

public:
    BallFilter(float smoothingFactor = 0.2f,float max_dist = 0.3f);
    Coord processNewBall(VisionResult ball);//adds a new Ball, and returns filtered result
};
