#pragma once

#include <array>
#include <cstring>
#include <optional>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/lock_options.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <ipc_sensor_message_generated.h>
#include <libbembelbots/bembelbots_shm.h>
#include <framework/ipc/shared_memory.h>
#include <representations/flatbuffers/types/sensors.h>

class MonitorShmContent {
public:
    static constexpr int NUM_JOINTS{25};
    using ipc_lock = boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>;

    // backend shall lock this mutex when writing
    boost::interprocess::interprocess_mutex mtx;

    BembelbotsShmFlatbuffer<bbapi::BembelIpcSensorMessage> sensors;
};

class MonitorShm {
public:
    explicit MonitorShm(const bool create = false) : shm("bbmonitorshm", create) {}

    void updateSensors(const bbapi::BembelIpcSensorMessageT &sensors) {
        // never blocks, either lock can be aquired or update is skipped
        MonitorShmContent::ipc_lock lock(shm->mtx, boost::interprocess::try_to_lock);
        if (!lock)
            return;

        shm->sensors.set(sensors);
    }

    // returns copy of shm content
    std::optional<bbapi::BembelIpcSensorMessageT> read() {
        bbapi::BembelIpcSensorMessageT sensors;
        // blocks until lock can be aquired
        MonitorShmContent::ipc_lock lock(shm->mtx);
        bool success = shm->sensors.get(sensors);
        if (success)
            return sensors;
        return std::nullopt;
    }

private:
    ipc::SharedMemory<MonitorShmContent> shm;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
