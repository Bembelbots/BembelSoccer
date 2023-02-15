#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <bodycontrol/simulator/simulator.hpp>
#include <bodycontrol/utils/special_stances.hpp>
#include <core/logger.h>

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

void runMotion(BodyControlSim *bcSim, MODULE_ID motion, std::ostream &os){
    BodyControl *bc = &bcSim->bc;
    BBActuatorData *actuatorData = &bcSim->actuatorData;

    bc->activateModule(motion);

    bool run = true;

    while(run){
        usleep(10000);

        printActuators(os, actuatorData);

        BodyState b = bcSim->step({});

        run = b.activeMotion == motion;
    }
}

std::string createHeader(std::vector<std::string> vec){
    std::stringstream ss;

    for(size_t i = 0; i < vec.size(); ++i)
    {
      if(i != 0)
        ss << ",";
      ss << vec[i];
    }

    ss << std::endl;

    return ss.str();
}

void openCSVFile(std::ofstream &fs, std::string path, const std::string &header = ""){
    fs.open(path);
    if(not header.empty())
        fs << header;
}

int main(int argc, const char* argv[]){
    fprintf(stderr, "libBembelBots: Start.\n");

    XLogger::quick_init("system");

    boost::filesystem::path full_path( boost::filesystem::initial_path<boost::filesystem::path>() );

    full_path = boost::filesystem::system_complete( boost::filesystem::path( argv[0] ) );

    std::string outdir = full_path.parent_path().string() + "/motions/";

    boost::filesystem::remove_all(outdir);

    boost::filesystem::create_directory(outdir);

    std::string header = createHeader({
        "Timestamp",
        "HeadYaw",
        "HeadPitch",
        "LShoulderPitch",
        "LShoulderRoll",
        "LElbowYaw",
        "LElbowRoll",
        "LWristYaw",
        "LHand",
        "RShoulderPitch",
        "RShoulderRoll",
        "RElbowYaw", 
        "RElbowRoll",
        "RWristYaw",
        "RHand", 
        "HipYawPitch",
        "LHipRoll",
        "LHipPitch",
        "LKneePitch",
        "LAnklePitch",
        "LAnkleRoll",
        "RHipRoll",
        "RHipPitch",
        "RKneePitch",
        "RAnklePitch",
        "RAnkleRoll",
    });

    ofstream outfile;

    BodyControlSim bcSim;
    bcSim.setStance<All>(SIT_STANCE);

    LOG_INFO << "Starting stand motion.";

    openCSVFile(outfile, outdir + "stand.csv", header);
    runMotion(&bcSim, STAND, outfile);
    outfile.close();

    LOG_INFO << "Finished stand motion.";

    LOG_INFO << "Starting sit motion.";

    openCSVFile(outfile, outdir + "sit.csv", header);
    runMotion(&bcSim, SIT, outfile);
    outfile.close();
    
    LOG_INFO << "Finished sit motion.";
}
