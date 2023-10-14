#include <iostream>
#include <cmath>
#include <cstring>
#include <bodycontrol/simulator/simulator.hpp>
#include <bodycontrol/utils/special_stances.hpp>
using namespace std;


#define TEXT_NORMAL "\033[0m"
#define TEXT_BOLD_RED "\033[1;31m"
#define TEXT_BOLD_YELLOW "\033[1;33m"
#define TEXT_BOLD_GREEN "\033[1;32m"
#define TEXT_BOLD_BLUE "\033[1;34m"
#define TEXT_RED "\033[0;31m"
#define TEXT_YELLOW "\033[0;33m"
#define TEXT_GREEN "\033[0;32m"
#define TEXT_BLUE "\033[0;34m"
#define TEXT_HEADLINE TEXT_RED
#define TEXT_SEPARATOR TEXT_BOLD_BLUE
#define TEXT_DATA TEXT_YELLOW
#define TEXT_PASSED TEXT_BOLD_GREEN
#define TEXT_FAILED TEXT_BOLD_RED
#define EL TEXT_NORMAL << std::endl
#define TN EL

#define INFO std::cout << "[INFO] " << TEXT_NORMAL
#define WARN std::cout << "[WARN] " << TEXT_BOLD_YELLOW
#define FAIL std::cout << "[FAIL] " << TEXT_RED
#define HEAD std::cout << "[HEAD] " << TEXT_BOLD_BLUE
#define GOOD std::cout << "[GOOD] " << TEXT_BOLD_GREEN
#define COMMENT std::cout << TEXT_YELLOW

void printActuators(BBActuatorData *actuatorData){
    INFO << "===========Actuators=============" << EL;
    for (int i = 0; i < lbbNumOfPositionActuatorIds; ++i) {
        std::cout << actuatorData->actuators[i] << std::endl;
    }
    INFO << "===========END Actuators=========" << EL;
}

void runMotion(BodyControlSim *bcSim, MODULE_ID motion, int interval=10000){
    int n = 0;
    float r_sum = 0.0f;
    struct timespec last_time; clock_gettime(CLOCK_MONOTONIC, &last_time);

    BodyControl *bc = &bcSim->bc;
    BBActuatorData *actuatorData = &bcSim->actuatorData;

    bc->issueCommand(motion);

    bool run = true;

    while(run){
        usleep(interval);

        // Timing //
        struct timespec cycle_time; clock_gettime(CLOCK_MONOTONIC, &cycle_time);
        long last_r=    (cycle_time.tv_nsec - last_time.tv_nsec)/1E3
                       +(cycle_time.tv_sec  - last_time.tv_sec )*1E6;

        r_sum += last_r ;
        n += 1;

        if(last_r > 2*interval){
            FAIL
                << "\n\t\t\tread avg: " << (r_sum/n)
                << "\n\t\t\tlast wait: " << last_r
            << EL;
        }

        printActuators(actuatorData);

        clock_gettime(CLOCK_MONOTONIC, &last_time);

        bcSim->step();

        CognitionMessage cm = bc->getCognitionMessage(0);

        run = cm.activeMotion == motion;
    }
}

int main(){
    fprintf(stderr, "libBembelBots: Start.\n");

    BodyControlSim bcSim;
    bcSim.setStance<Legs>(SIT_STANCE);

    INFO << "=====MOTION_STAND=====" << EL;

    runMotion(&bcSim, STAND);

    INFO << "=====MOTION_SIT=====" << EL;

    runMotion(&bcSim, SIT);

    return 0;
}