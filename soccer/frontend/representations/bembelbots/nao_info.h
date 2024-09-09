//
// Created by kito on 4/26/18.
//

#pragma once
#include "ipc_sensor_message_generated.h"
#include <string>
#include <utility>
#include <libbembelbots/bembelbots_shm.h>

struct NaoInfo {
    explicit NaoInfo() = default;

    std::string getName() const;
    std::string getHostname() const;

    bbapi::RobotName getNameId() const;

    bbapi::RobotName robotName = bbapi::RobotName::UNKNOWN;
    std::string robotNameStr = "unknown";
};


enum class NaoInfoReaderError {
    NONE = 0,
    BACKEND_CONNECT_ERROR,
    BACKEND_READ_ERROR
};

struct NaoInfoResult {
    NaoInfo naoInfo;
    NaoInfoReaderError error{NaoInfoReaderError::NONE};
};

class NaoInfoReader {
public:
    explicit NaoInfoReader() = default;
    NaoInfoResult getNaoInfo(bool docker = false);
private:
    using ipcType = ipc::TrippleBuffer<BembelbotsShmFlatbuffer<bbapi::BembelIpcSensorMessage>>;
    bool getSensors(ipcType &reader, bbapi::BembelIpcSensorMessageT &sensorMsg);
    int failCount = 0;
};
