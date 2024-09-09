#pragma once
#include <memory>
#include <representations/bembelbots/types.h>
#include "walk_calibration.h"

struct DirectedCoord;
struct BodyBlackboard;

namespace detail {
    struct WalkingEngineImpl;
}

class WalkingEngine {
public:
    WalkingEngine();
    void reset();
    void proceed(BodyBlackboard *);
    
    bool isStanding();

    void setDirection(DirectedCoord);
    void setBodyHeight(BodyHeight);
    void setStiffness(float);
    void setStand(bool);

    float getSupportFoot();

private:
    friend detail::WalkingEngineImpl;
    std::shared_ptr<detail::WalkingEngineImpl> impl;

    WalkCalibration calibration;
    
    static constexpr float bodyHeightHighStand = 0.203f;
};
