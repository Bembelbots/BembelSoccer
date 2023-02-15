//
// Created by kito on 4/26/18.
//

#pragma once
#include <string>
#include <utility>
#include <libbembelbots/bembelbots.h>

namespace ipc {
template<typename T>
class TrippleBuffer;
}

class BBSensorData;


struct NaoInfo {
    explicit NaoInfo() = default;

    std::string getName() const;
    std::string getHostname() const;

    RobotName getNameId() const;
    BackendType getBackendType() const;
    RobotVersion getRobotVersion() const;
    RobotType getRobotType() const;

    RobotName robotName = RobotName::UNKNOWN;
    BackendType backendType = BackendType::BackendType_Naoqi;
    std::string robotNameStr = "unknown";
    int instance;
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
    NaoInfoResult getNaoInfo(const int &instance, bool docker = false);
private:
    bool getSensors(ipc::TrippleBuffer<BBSensorData> &reader, BBSensorData *data);
    int failCount = 0;
};
