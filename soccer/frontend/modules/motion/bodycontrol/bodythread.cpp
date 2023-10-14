#include "bodythread.h"
#include <core/logger.h>
#include <core/util/niceness.h>
#include <core/settingsblackboard.h>
#include <unistd.h>

const std::string BodyThread::threadName = "BodyThread";

static constexpr int MAX_SENSOR_FETCH_MS = 13; //< Warn when getting sensors takes longer than this

BodyThread::BodyThread() : bc(), sensorData(), actuatorData() {
}

void BodyThread::connect(rt::Linker &link) {
    link.name = "body";

    link(settings);
    link(cmds);
    link(state);

    bc.connect(link);
}

void BodyThread::setup() {
    nice(NICENESS_VERY_HIGH_PRIO);

    int backendInstance = settings->instance;

    std::stringstream shmName;

    shmName << bembelbotsShmName << "_" << backendInstance;

    LOG_INFO << "BembelbotsShm: " << shmName.str();

    shm = std::make_unique<BembelbotsShm>(shmName.str(), false);

    ticks = 0;
    missedFrames = 0;

    timeout = MAX_SENSOR_FETCH_MS;
    if (settings->simulator)
        timeout = 1000;

    SubModule::Setup bcSetup{settings, &cmds};
    bc.setup(bcSetup);
}

void BodyThread::process() {

    ///*************///
    ///*************///
    ///* MAIN LOOP *///
    ///*************///
    ///*************///

    /*********************************/
    /* Fetch Sensors and Synchronize */
    /*********************************/
    if (!(*shm)->sensors.timedProduce(timeout)) {
        LOG_WARN_EVERY_N(50) << "Fetching sensors took too long... (missedFrames = " << missedFrames << ")";
        missedFrames++;
    }

    jsassert((*shm)->sensors.producedData().backendVersion == CURRENT_BACKEND_VERSION)
            << "\n\e[1;31m !!!  Incompatible backend version, update backend !!! \e[0m";

    LOG_INFO_EVERY_N(500) << "BodyThread is alive...";

    /********************/
    /* BodyControl Step */
    /********************/
    *state = bc.step(cmds, &(*shm)->actuators.consumedData(), &(*shm)->sensors.producedData());

    /************************/
    /* Write Actuators Back */
    /************************/
    (*shm)->actuators.consumedData().tick = ticks++;
    (*shm)->actuators.consume();
}

void BodyThread::stop(){
    LOG_INFO << "BodyControl::stop()";
}
