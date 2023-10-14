#pragma once

#include <framework/joints/joints.hpp>
// #include <bodycontrol/htwk/walk/ankle_balancer_com.h>
// #include <bodycontrol/htwk/walk/shoulder_balancer.h>
#include "combined_balancer.h"

#include <iosfwd>
#include <string>
#include <vector>

// todo: best practice for this?
/* struct FromFileType {};
constexpr FromFileType fromFile;

struct FromStringType {};
constexpr FromStringType fromString; */
#include <bodycontrol/utils/FromWhichType.h>


// todo: understand abbreviation (bbmf) and rename this class accordingly
//          also rename file suffix (currently .rrmf)
class MotionFile {
public:
    static std::string motion_path;

    struct frameDetails {
        joints::pos::Old j;
        int t;
        std::vector<float> d;
    };

    //BBMF(const std::string &filename, bool useAnkleBalancer=false);
    MotionFile(FromFileType, const std::string &fileName, bool useLinearInterpol=false);
    MotionFile(FromStringType, const std::string &bbmfStr);

    ~MotionFile();

    void step(int currentTime, joints::Actuators &, const joints::Sensors &, 
            float gyroX=0.f, float gyroY=0.f);

    void reset();

    std::vector<frameDetails> getFrames() const { 
        return frames; 
    }

    int getDuration() const;

    int getCurrentTick() const;

    bool isActive() const;

private:
    // std::vector<std::pair<joints::pos::Old, int>> frames;


    std::vector<frameDetails> frames;

    joints::Linear<joints::pos::Old> interpolationLinear;
    joints::Cubic<joints::pos::Old> interpolation;

    uint maxDuration = 0;
    uint currentFrame = 0;
    int startTime = 0;

    bool interpolating = false;

    // AnkleBalancerCoM ankleBalancer;
    bool useAnkleBalancer = false;

    // ShoulderBalancer shoulderBalancer;
    bool useShoulderBalancerPitch = false;
    bool useShoulderBalancerRoll = false;

    CombinedBalancer combinedBalancer;

    bool useLinearInterpol = false;

    int _nextPoseReached = -1;
    int _currentTick = 0;


    //read from file
    bool loadFromMotionfile(const std::string &filename);
    bool loadStream(std::istream &);
    bool calculateInterpolationParams(const joints::Sensors &);

    joints::Cubic<joints::pos::Old> applyTransition(const joints::pos::Old &start, 
                                        const joints::pos::Old &target, 
                                        const int &timeTicks, 
                                        const float &duration);

    joints::pos::Old calculateNextTick();

    void printData();
};
