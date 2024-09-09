#pragma once

#include <framework/joints/joints.hpp>
#include "../../walk/htwk/ankle_balancer.h"
#include <bodycontrol/utils/FromWhichType.h>


#include <iosfwd>
#include <string>
#include <vector>


class BBMF {
public:
    static std::string bbmf_path;

    BBMF(FromFileType, const std::string &fileName,
            bool useLinearInterpol=false,
            bool useAnkleBalancer=false,
            bool delaySetup = false);

    BBMF(FromStringType, const std::string &bbmfStr);

    ~BBMF();

    void setup();

    void step(int currentTime, bbipc::Actuators *, const bbipc::Sensors &, 
            float gyroX=0.f, float gyroY=0.f);

    void reset();

    std::vector<std::pair<joints::pos::Old, int>> getFrames() const { 
        return frames; 
    }

    int getDuration() const;

    int getCurrentTick() const;

    bool isActive() const;

private:
    std::vector<std::pair<joints::pos::Old, int>> frames;

    joints::Linear<joints::pos::Old> interpolationLinear;
    joints::Cubic<joints::pos::Old> interpolation;
    
    const std::string fileName;

    uint maxDuration = 0;
    uint currentFrame = 0;
    int startTime = 0;

    bool setupComplete = false;
    bool interpolating = false;

    AnkleBalancer ankleBalancer;
    bool useAnkleBalancer = false;

    bool useLinearInterpol = false;

    int _nextPoseReached = -1;
    int _currentTick = 0;


    //read from file
    bool loadFromMotionfile(const std::string &filename);
    bool loadStream(std::istream &);
    bool calculateInterpolationParams(const bbipc::Sensors &);

    joints::Cubic<joints::pos::Old> applyTransition(const joints::pos::Old &start, 
                                        const joints::pos::Old &target, 
                                        const int &timeTicks, 
                                        const float &duration);

    joints::pos::Old calculateNextTick();

    void printData();
};
