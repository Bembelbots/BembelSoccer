#include <bodycontrol/motionrunner.h>
#include <bodycontrol/utils/bbmf.h>
#include <bodycontrol/simulator/simulator.hpp>
#include <bodycontrol/environment/logger.h>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

void printActuators(std::ostream &os, BBActuatorData *actuatorData){
    const int precision = 12;

    milliseconds timestamp = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    );

    os << timestamp.count();

    for (int i = 0; i < lbbNumOfPositionActuatorIds; ++i) {
        os << ',' << std::fixed << std::setprecision(precision) 
          << std::setfill( '0' ) << actuatorData->actuators[i];
    }

    os << std::endl;
}


int main(int argc, const char* argv[]){
    std::string baseDir(getenv("AL_DIR"));

    if (baseDir.empty()) {
        LOG_ERROR << "AL_DIR not set";
        exit(1);
    }

    BBMF::bbmf_path = baseDir + "/motions";

    BBSensorData sensorData;
    BBActuatorData actuatorData;

    memset(&actuatorData, 0, sizeof(BBActuatorData)); // cppcheck-suppress memsetClassFloat
    memset(&sensorData, 0, sizeof(BBSensorData)); // cppcheck-suppress memsetClassFloat

    MotionRunner runner;
    runner.setMotion(STAND_UP_FROM_BACK);

    BodyBlackboard *bb = &runner.bodyblackboard;

    struct timespec now, start_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int count = 0;
    while(runner.isActive()){
        clock_gettime(CLOCK_MONOTONIC, &now);
        int timestamp_ms = ((now.tv_nsec-start_time.tv_nsec)/1E6)
            +((now.tv_sec-start_time.tv_sec)*1E3);
        bb->timestamp_ms = timestamp_ms;
        runner.step(actuatorData.actuators, sensorData.sensors);
        count++;
        usleep(10000);
    }

    LOG_INFO << count << std::endl;
}
