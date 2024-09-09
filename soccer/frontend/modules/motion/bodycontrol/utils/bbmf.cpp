#include "bbmf.h"
#include "framework/joints/body_v6.h"
#include "framework/joints/joints.hpp"

#include <framework/logger/logger.h>

#include <fstream>
#include <sstream>


using namespace joints;


std::string BBMF::bbmf_path = "";


BBMF::BBMF(FromFileType, const std::string &fileName,
        bool useLinearInterpol,
        bool useAnkleBalancer,
        bool delaySetup)
    : fileName(fileName) {
    this->useLinearInterpol = useLinearInterpol;
    this->useAnkleBalancer = useAnkleBalancer;

    if(not delaySetup) {
        setup();
    }
}

BBMF::BBMF(FromStringType, const std::string &bbmfStr) {
    std::istringstream ss(bbmfStr);

    if (not loadStream(ss)) {
        LOG_ERROR << "Cannot load BBMF: " << bbmfStr;
    }
}

BBMF::~BBMF() {}

void BBMF::setup() {
    if(setupComplete) {
        return;
    }
    setupComplete = true;

    if (not loadFromMotionfile(fileName)) {
        LOG_ERROR << "Cannot load requested (BBMF-)motion from file: " << fileName;
    }
}

int BBMF::getDuration() const {
    return maxDuration;
}

void BBMF::step(int currentTime, bbipc::Actuators *actuators, 
        const bbipc::Sensors &sensors, float gyroX, float gyroY){
    _currentTick = currentTime;

    if (_currentTick > _nextPoseReached 
            && !calculateInterpolationParams(sensors)) {
        return;
    }
    
    joints::pos::Old nextTick = calculateNextTick();


    if(useAnkleBalancer){
        YPR ypr(0.0f, gyroY, gyroX);
        ankleBalancer.proceed(ypr); 

        pos::Legs ankles;
        ankles[JointNames::LAnklePitch] = ankles[JointNames::RAnklePitch] = ankleBalancer.pitch * 0.5f;
        ankles[JointNames::LAnkleRoll] = ankles[JointNames::RAnkleRoll] = ankleBalancer.roll * 0.5f;
        nextTick += ankles;
    }

    nextTick.write(actuators);
}

void BBMF::reset() {
    currentFrame = 0;
    _currentTick = 0;
    _nextPoseReached = -1;
}

bool BBMF::isActive() const {
    return currentFrame < frames.size() or _currentTick < _nextPoseReached;
}

int BBMF::getCurrentTick() const {
    return _currentTick;
}

/*
    @TODO Refactor

    Loads a bbmf file and seperating the actuator data in degrees and the interpolation time in ms.
    The actuator data is converted to radians and reordered to match the DCM order.
*/

bool BBMF::loadFromMotionfile(const std::string &filename) {
    static const std::string suffix = ".bbmf";
    static const int numberOfActuators = 21;

    if (filename.find(suffix, 0) + suffix.size() != filename.size()) {
        LOG_WARN << filename << " does not have bbmf file ending!"; 
        return false;
    }

    std::string fn = bbmf_path + "/" + filename;
    std::ifstream motionfile(fn.c_str());

    // error while loading file
    if (not motionfile.is_open()) {
        LOG_ERROR << "could not load or find motionfile with name " << fn;
        return false;
    }

    return loadStream(motionfile);
}

bool BBMF::loadStream(std::istream &ss) {

    static const int maxLineValues = 24;

    std::vector<int> values;
    std::string strval;
    std::string line; 
    int intval;

    bool valid = true;
    while (std::getline(ss, line)) {
        if ((not line.size()) || (line[0] == '#') || (line[0] == ':')) {
            continue;
        }

        values.clear();

        for (unsigned int i = 0; i <= line.size(); ++i) {
            if (values.size() > maxLineValues) {
                LOG_WARN << "BBMF::loadFromMotionFile: more than " << maxLineValues << " values!"; 
                valid = false;
                break;
            }

            // detect delimiter and convert string to integer
            // and insert it into array of ints
            if ((i == line.size()) || (line[i] == ' ') || (line[i] == ',')) {
                // replace * with 255
                // this value will be replaced later with the current motor value
                // we could do this here, but then we have to calc back
                // the float to integer. very bad...
                if (strval == "*") {
                    values.push_back(255);
                } else {
                    std::istringstream ssval(strval);
                    ssval >> intval;
                    values.push_back(intval);
                }
                strval.clear();
            } else {
                strval += line[i];
            }
        }

        if (values.size() != maxLineValues) {
            LOG_ERROR << "failed to insert new motion line due to wrong value count!"; 
            valid = false;
            break;
        }

        {
            pos::Old actuators(Joints{
                .headYaw = float(values[0]), // HEAD_YAW
                .headPitch = float(values[1]), // HEAD_PITCH
                .lShoulderPitch = float(values[2]), // LEFT_SHOULDER_PITCH
                .lShoulderRoll = float(values[3]), // LEFT_SHOULDER_ROLL
                .lElbowYaw = float(values[4]), // LEFT_ELBOW_YAW
                .lElbowRoll = float(values[5]), // LEFT_ELBOW_ROLL
                .lWristYaw = 0,

                .hipYawPitch = float(values[6]), // HIP_YAW_PITCH

                .lHipRoll = float(values[7]), // LEFT_HIP_ROLL
                .lHipPitch = float(values[8]), // LEFT_HIP_PITCH
                .lKneePitch = float(values[9]), // LEFT_KNEE_PITCH
                .lAnklePitch = float(values[10]), // LEFT_ANKEL_PITCH
                .lAnkleRoll = float(values[11]), // LEFT_ANKEL_ROLL

                // 12 left out due to double HIP_YAW_PITCH

                .rHipRoll = float(values[13]), // RIGHT_HIP_ROLL
                .rHipPitch = float(values[14]), // RIGHT_HIP_PITCH
                .rKneePitch = float(values[15]), // RIGHT_KNEE_PITCH
                .rAnklePitch = float(values[16]), // RIGHT_ANKEL_PITCH
                .rAnkleRoll = float(values[17]), // RIGHT_ANKEL_ROLL

                .rShoulderPitch = float(values[18]), // RIGHT_SHOULDER_PITCH
                .rShoulderRoll = float(values[19]), // RIGHT_SHOULDER_ROLL

                .rElbowYaw = float(values[20]), // RIGHT_ELBOW_YAW
                .rElbowRoll = float(values[21]), // RIGHT_ELBOW_ROLL

                .rWristYaw = 0,

                .lHand = 0,
                .rHand = 0,
            });

            actuators *= DEG_TO_RAD;

            int duration = values.at(23);
            maxDuration += duration / 10;
            frames.push_back({actuators, duration});
        }
    }

    return valid;
}

bool BBMF::calculateInterpolationParams(const bbipc::Sensors &sensors) {
    pos::Old prev;

    if (currentFrame == frames.size()) {
        return false;
    }

    if (currentFrame == 0) {
        prev.read(sensors);
    } else {
        prev = frames.at(currentFrame - 1).first;
    }

    auto current = frames.at(currentFrame);
    if(useLinearInterpol){
        interpolationLinear = Linear<pos::Old>(prev, current.first, 
            _currentTick, current.second);
    } else{
        interpolation = Cubic<pos::Old>(prev, current.first, 
            _currentTick, current.second);    
    }
    _nextPoseReached = _currentTick + current.second;
    ++currentFrame;

    return true;
}

pos::Old BBMF::calculateNextTick() {
    if(useLinearInterpol){
        return interpolationLinear.get(_currentTick);
    }
    return interpolation.get(_currentTick);
}

void BBMF::printData(){
    for (auto frame : frames) {
        LOG_INFO << frame.first;
        LOG_INFO << "duration: " << frame.second; 
    }
}
