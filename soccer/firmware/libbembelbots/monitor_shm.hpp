#pragma once

#include <array>
#include <cstring>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/lock_options.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <framework/ipc/shared_memory.h>
#include "bembelbots.h"

class MonitorShmContent {
public:
    static constexpr int NUM_JOINTS{25};
    using ipc_lock =
        boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>;
    using joint_array = std::array<float, NUM_JOINTS>;

    // backend shall lock this mutex when writing
    boost::interprocess::interprocess_mutex mtx;

    RobotName name{RobotName::UNKNOWN};

    char headSerial[30] {""};
    char headVersion[10] {""};
    char bodySerial[30] {""};
    char bodyVersion[10] {""};

    joint_array jointTemp{0};
    joint_array jointStatus{0};

    float batCharge{-1};
    float batStatus{-1};
    float batCurrent{-1};
    float batTemp{-1};

    MonitorShmContent() {
        bzero(headSerial, sizeof(headSerial));
        bzero(headVersion, sizeof(headVersion));
        bzero(bodySerial, sizeof(bodySerial));
        bzero(bodyVersion, sizeof(bodyVersion));
    }
    
    // default copy constructor has been deleted
    MonitorShmContent(const MonitorShmContent &other):
        MonitorShmContent() {
        name = other.name;
        
        std::memcpy(headSerial, other.headSerial, sizeof(headSerial)-1);
        std::memcpy(headVersion, other.headVersion, sizeof(headVersion)-1);
        std::memcpy(bodySerial, other.bodySerial, sizeof(bodySerial)-1);
        std::memcpy(bodyVersion, other.bodyVersion, sizeof(bodyVersion)-1);

        jointTemp = other.jointTemp;
        jointStatus = other.jointStatus;
       
        batCharge = other.batCharge;
        batStatus = other.batStatus;
        batCurrent = other.batCurrent;
        batTemp = other.batTemp;
    }
};


class MonitorShm {
public:
    MonitorShm(const bool create=false): shm("bbmonitorshm", create) {}


    void updateRobotInfo(const RobotName &name,
            const std::string &headSerial,
            const std::string &headVersion,
            const std::string &bodySerial,
            const std::string &bodyVersion) {
        // waits for lock
        MonitorShmContent::ipc_lock lock(shm->mtx);
        
        shm->name = name;
        std::strncpy(shm->headSerial, headSerial.c_str(), sizeof(shm->headSerial)-1);
        std::strncpy(shm->headVersion, headVersion.c_str(), sizeof(shm->headVersion)-1);
        std::strncpy(shm->bodySerial, bodySerial.c_str(), sizeof(shm->bodySerial)-1);
        std::strncpy(shm->bodyVersion, bodyVersion.c_str(), sizeof(shm->bodyVersion)-1);
    }


    void updateSensors(const std::array<float, lbbNumOfSensorIds> &sensors) {
        // never blocks, either lock can be aquired or update is skipped
        MonitorShmContent::ipc_lock lock(shm->mtx, boost::interprocess::try_to_lock);
        if (!lock)
            return;

        shm->jointTemp = {
            sensors[headPitchTemperatureSensor],
            sensors[headYawTemperatureSensor],
            sensors[lAnklePitchTemperatureSensor],
            sensors[lAnkleRollTemperatureSensor],
            sensors[lElbowRollTemperatureSensor],
            sensors[lElbowYawTemperatureSensor],
            sensors[lHipPitchTemperatureSensor],
            sensors[lHipRollTemperatureSensor],
            sensors[lHipYawPitchTemperatureSensor],
            sensors[lKneePitchTemperatureSensor],
            sensors[lShoulderPitchTemperatureSensor],
            sensors[lShoulderRollTemperatureSensor],
            sensors[rAnklePitchTemperatureSensor],
            sensors[rAnkleRollTemperatureSensor],
            sensors[rElbowRollTemperatureSensor],
            sensors[rElbowYawTemperatureSensor],
            sensors[rHipPitchTemperatureSensor],
            sensors[rHipRollTemperatureSensor],
            sensors[rKneePitchTemperatureSensor],
            sensors[rShoulderPitchTemperatureSensor],
            sensors[rShoulderRollTemperatureSensor],
            sensors[lWristYawTemperatureSensor],
            sensors[rWristYawTemperatureSensor],
            sensors[lHandTemperatureSensor],
            sensors[rHandTemperatureSensor]
        };

        shm->batCharge = sensors[batteryChargeSensor];
        shm->batCurrent = sensors[batteryCurrentSensor];
        shm->batTemp = sensors[batteryTemperatureSensor];
    }


    void updateJointTemp(const MonitorShmContent::joint_array &a) {
        // never blocks, either lock can be aquired or update is skipped
        MonitorShmContent::ipc_lock lock(shm->mtx, boost::interprocess::try_to_lock);
        if (!lock)
            return;

        shm->jointTemp = a;
    }


    void updateJointStatus(const MonitorShmContent::joint_array &a) {
        // never blocks, either lock can be aquired or update is skipped
        MonitorShmContent::ipc_lock lock(shm->mtx, boost::interprocess::try_to_lock);
        if (!lock)
            return;

        shm->jointStatus = a;
    }


    void updateBattery(const std::array<float, 4> &a) {
        // never blocks, either lock can be aquired or update is skipped
        MonitorShmContent::ipc_lock lock(shm->mtx, boost::interprocess::try_to_lock);
        if (!lock)
            return;

        shm->batCharge  = a[0];
        shm->batStatus  = a[1];
        shm->batCurrent = a[2];
        shm->batTemp    = a[3];
    }


    // returns copy of shm content
    MonitorShmContent read() {
        // blocks until lock can be aquired
        MonitorShmContent::ipc_lock lock(shm->mtx);
        return *shm;
    }

private:
    ipc::SharedMemory<MonitorShmContent> shm;
};


// vim: set ts=4 sw=4 sts=4 expandtab:
