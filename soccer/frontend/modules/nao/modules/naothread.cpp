#include "naothread.h"
#include "ipc_sensor_message_generated.h"
#include "representations/flatbuffers/types/actuators.h"
#include "representations/flatbuffers/types/sensors.h"
#include <framework/logger/logger.h>
#include <framework/util/niceness.h>
#include <representations/blackboards/settings.h>
#include <framework/util/clock.h>
#include <framework/util/clock_simulator.h>
#include <representations/bembelbots/constants.h>

#include <ipc_sensor_message_generated.h>
#include <ipc_actuator_message_generated.h>

const std::string NaoThread::threadName = "NaoThread";

void NaoThread::set_sensor_callback(SensorCallback callback) {
    sensor_callback = callback;
}

void NaoThread::connect(rt::Linker &link) {
    link.name = "nao";
    link(settings);
    link(nao_state);
    link(nao_info);
    link(actuatorMsg);
    link(sensorMsg);
}

void NaoThread::setup() {
    nice(NICENESS_VERY_HIGH_PRIO);

    if (!sensor_callback) {
        LOG_ERROR << "Sensor callback not set!";
        LOG_FLUSH();
        exit(EXIT_FAILURE);
    }

    LOG_INFO << "BembelbotsShm: " << bembelbotsShmName;

    try {
        shm = std::make_unique<BembelbotsShm>(bembelbotsShmName, false);
    } catch (const boost::interprocess::interprocess_exception &ex) {
        if (ex.get_error_code() == boost::interprocess::error_code_t::not_found_error) {
            LOG_ERROR << "Backend is not running";
        }
        LOG_FLUSH();
        exit(EXIT_FAILURE);
    }

    ticks = 0;
    missedFrames = 0;

    timeout = MAX_SENSOR_FETCH_MS;
    if (settings->simulator)
        timeout = 1000;
}

void NaoThread::process() {
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

    auto &sipc{(*shm)->sensors.producedData()};
    jsassert(sipc.get(sensorMsg)) << "Invalid actuator message flatbuffer received!";

    sensorMsg->timestamp = getTimestampMs();

    jsassert(sensorMsg->backendVersion == BB_BACKEND_VERSION)
            << "\n\e[1;31m !!!  Incompatible backend version, update backend !!! \e[0m";

    NaoState current_state{.timestamp_ms = getTimestampMs(),
            .lola_timestamp = sensorMsg->lolaTimestamp,
            .connected = sensorMsg->connected,
            .bCamPose = {},
            .tCamPose = {},
            .actuatorData = actuatorMsg,
            .sensorData = sensorMsg};

    if (settings->simulator) {
        const auto &simTick{sensorMsg->sensors.battery.temperature};
        if (settings->simulator && (simTick > 0)) {
            current_state.lola_timestamp = current_state.timestamp_ms = simTick * CONST::lola_cycle_ms;
            setGlobalTimeFromSimulation(current_state.timestamp_ms);
        }
    }

    sensor_callback(current_state);

    *nao_state = current_state;

    /************************/
    /* Write Actuators Back */
    /************************/
    actuatorMsg->tick = ticks++;

    auto &aipc{(*shm)->actuators.consumedData()};
    jsassert(aipc.set(actuatorMsg)) << "Actuator message flatbuffer exceeds SHM size!";
    (*shm)->actuators.consume();
}

void NaoThread::stop() {
    LOG_INFO << "nao thread stopped";
}
