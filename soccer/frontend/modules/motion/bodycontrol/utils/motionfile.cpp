#include "motionfile.h"

#include <framework/logger/logger.h>

#include <fstream>
#include <sstream>
#include <cassert>


using namespace joints;

std::string MotionFile::motion_path = "";


MotionFile::MotionFile(FromFileType, const std::string &fileName, bool useLinearInterpol) {
    if (not loadFromMotionfile(fileName)) {
        LOG_ERROR << "Cannot load requested motion from .mf-file: " << fileName;
    } else LOG_DEBUG << "Loading from .mf-file " << fileName << " with " << frames.size() << " frames was succesful.";
    this->useLinearInterpol = useLinearInterpol;
}

MotionFile::MotionFile(FromStringType, const std::string &bbmfStr) {
    std::istringstream ss(bbmfStr);

    if (not loadStream(ss)) {
        LOG_ERROR << "Cannot load BBMFRAD: " << bbmfStr;
    }
}

MotionFile::~MotionFile() {}


int MotionFile::getDuration() const {
    return maxDuration;
}

void MotionFile::step(int currentTime, joints::Actuators &actuators, const joints::Sensors &sensors, float gyroX, float gyroY) {
    _currentTick = currentTime;

    if (_currentTick > _nextPoseReached && !calculateInterpolationParams(sensors)) {
        return;
    }
    
    joints::pos::Old nextTick = calculateNextTick();

    //LOG_INFO_EVERY_N(10000) << "useAnkleBalancer = " << useAnkleBalancer << " (should have been 1) " << useShoulderBalancer;

    // if(useAnkleBalancer){
        // ankleBalancer.proceed(gyroX, gyroY, frames.at(currentFrame).d);
        // shoulderBalancer.proceed(gyroX, gyroY, frames.at(currentFrame).d);
        combinedBalancer.proceed(gyroX, gyroY, frames.at(currentFrame).d);

        //LOG_DEBUG << "gyroX = " << gyroX << " gyroY = " << gyroY << " ankleBalancer: Pitch: " << ankleBalancer.getPitch() << " Roll: " << ankleBalancer.getRoll();

        pos::Old ankles(Joints{
                .headYaw = 0, // HEAD_YAW
                .headPitch = 0, // HEAD_PITCH
                .lShoulderPitch = combinedBalancer.getShoulderPitch(), // LEFT_SHOULDER_PITCH
                .lShoulderRoll = combinedBalancer.getShoulderRollL(), // LEFT_SHOULDER_ROLL
                .lElbowYaw = 0, // LEFT_ELBOW_YAW
                .lElbowRoll = 0, // LEFT_ELBOW_ROLL
                .lWristYaw = 0,
                .lHand = 0,

                .hipYawPitch = 0, // HIP_YAW_PITCH

                .lHipRoll = 0, // LEFT_HIP_ROLL
                .lHipPitch = 0, // LEFT_HIP_PITCH
                .lKneePitch = 0, // LEFT_KNEE_PITCH
                .lAnklePitch = combinedBalancer.getAnklePitch(), // LEFT_ANKEL_PITCH
                .lAnkleRoll = combinedBalancer.getAnkleRoll(), // LEFT_ANKEL_ROLL

                .rShoulderPitch = combinedBalancer.getShoulderPitch(), // RIGHT_SHOULDER_PITCH
                .rShoulderRoll = combinedBalancer.getShoulderRollR(), // RIGHT_SHOULDER_ROLL

                .rElbowYaw = 0, // RIGHT_ELBOW_YAW
                .rElbowRoll = 0, // RIGHT_ELBOW_ROLL

                .rWristYaw = 0,
                .rHand = 0,

                // 12 left out due to double HIP_YAW_PITCH

                .rHipRoll = 0, // RIGHT_HIP_ROLL
                .rHipPitch = 0, // RIGHT_HIP_PITCH
                .rKneePitch = 0, // RIGHT_KNEE_PITCH
                .rAnklePitch = combinedBalancer.getAnklePitch(), // RIGHT_ANKEL_PITCH
                .rAnkleRoll = combinedBalancer.getAnkleRoll(), // RIGHT_ANKEL_ROLL
            });

        nextTick += ankles;
    // }
    nextTick.write(actuators);
}

void MotionFile::reset() {
    currentFrame = 0;
    _currentTick = 0;
    _nextPoseReached = -1;
}

bool MotionFile::isActive() const {
    return currentFrame < frames.size() or _currentTick < _nextPoseReached;
}

int MotionFile::getCurrentTick() const {
    return _currentTick;
}

/*
    @TODO Refactor

    Loads a bbmf file and seperating the actuator data in degrees and the interpolation time in ms.
    The actuator data is converted to radians and reordered to match the DCM order.
*/

bool MotionFile::loadFromMotionfile(const std::string &filename) {
    static const std::string suffix = ".mf";
    // static const int numberOfActuators = 21;

    if (filename.find(suffix, 0) + suffix.size() != filename.size()) {
        LOG_WARN << filename << " does not have .mf file ending!"; 
        return false;
    }

    std::string fn = motion_path + "/" + filename;
    std::ifstream motionfile(fn.c_str());

    assert(motionfile.is_open());

    // error while loading file
    if (not motionfile.is_open()) {
        LOG_ERROR << "could not load or find motionfile with name " << fn << " Error: " << strerror(errno);
        return false;
    }

    return loadStream(motionfile);
}

bool MotionFile::loadStream(std::istream &ss) {

    static const int maxLineValues = 24 + 8 + 6 + 4;

    std::vector<float> values;
    std::string strval;
    std::string line; 
    //int intval;
    float floatval;

    bool valid = true;
    while (std::getline(ss, line)) {
        if ((not line.size()) || (line[0] == '#') || (line[0] == ':')) {
            continue;
        }

        values.clear();

        for (unsigned int i = 0; i <= line.size(); ++i) {
            if (values.size() > maxLineValues) {
                LOG_WARN << "BBMFRAD::loadFromMotionFile: more than " << maxLineValues << " values!"; 
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
                    values.push_back(255.0f); // todo: functionality not guaranteed!!
                } else {
                    std::istringstream ssval(strval);
                    ssval >> floatval;
                    values.push_back(floatval);
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
                .headYaw = values[0], // HEAD_YAW
                .headPitch = values[1], // HEAD_PITCH
                .lShoulderPitch = values[2], // LEFT_SHOULDER_PITCH
                .lShoulderRoll = values[3], // LEFT_SHOULDER_ROLL
                .lElbowYaw = values[4], // LEFT_ELBOW_YAW
                .lElbowRoll = values[5], // LEFT_ELBOW_ROLL
                .lWristYaw = 0,
                .lHand = 0,

                .hipYawPitch = values[6], // HIP_YAW_PITCH

                .lHipRoll = values[7], // LEFT_HIP_ROLL
                .lHipPitch = values[8], // LEFT_HIP_PITCH
                .lKneePitch = values[9], // LEFT_KNEE_PITCH
                .lAnklePitch = values[10], // LEFT_ANKEL_PITCH
                .lAnkleRoll = values[11], // LEFT_ANKEL_ROLL

                .rShoulderPitch = values[18], // RIGHT_SHOULDER_PITCH
                .rShoulderRoll = values[19], // RIGHT_SHOULDER_ROLL

                .rElbowYaw = values[20], // RIGHT_ELBOW_YAW
                .rElbowRoll = values[21], // RIGHT_ELBOW_ROLL

                .rWristYaw = 0,
                .rHand = 0,

                // 12 left out due to double HIP_YAW_PITCH

                .rHipRoll = values[13], // RIGHT_HIP_ROLL
                .rHipPitch = values[14], // RIGHT_HIP_PITCH
                .rKneePitch = values[15], // RIGHT_KNEE_PITCH
                .rAnklePitch = values[16], // RIGHT_ANKEL_PITCH
                .rAnkleRoll = values[17], // RIGHT_ANKEL_ROLL
            });

            //actuators *= DEG_TO_RAD;

            int duration = values.at(23);
            maxDuration += duration / 10;

            std::vector<float> balancingDetails;
            for (size_t i = 24; i < values.size(); i++) {
                balancingDetails.push_back(values[i]);
            }

            frames.push_back({actuators, duration, balancingDetails});
        }
    }

    return valid;
}

bool MotionFile::calculateInterpolationParams(const joints::Sensors &sensors) {
    pos::Old prev;
    assert(frames.size() != 0);

    if (currentFrame >= frames.size() -1) {    // ==  -1
        if (currentFrame == frames.size()) 
            return false;
        else {
            ++currentFrame;     //why does this fix the not ending motion problem???
            return false;
        }
    }

    if (currentFrame == 0) {
        prev.read(sensors);
    } else {
        prev = frames.at(currentFrame - 1).j;
    }

    auto current = frames.at(currentFrame);
    if (useLinearInterpol) {
        interpolationLinear = Linear<pos::Old>(prev, current.j,  _currentTick, current.t);
    } else {
        interpolation = Cubic<pos::Old>(prev, current.j, _currentTick, current.t);    
    }
    _nextPoseReached = _currentTick + current.t;
    ++currentFrame;

    return true;
}

pos::Old MotionFile::calculateNextTick() {
    if (useLinearInterpol) {
        return interpolationLinear.get(_currentTick);
    }
    else {
        return interpolation.get(_currentTick);
    }
}

void MotionFile::printData(){
    for (auto frame : frames) {
        LOG_INFO << frame.j;
        LOG_INFO << "duration: " << frame.t;
    }
}
