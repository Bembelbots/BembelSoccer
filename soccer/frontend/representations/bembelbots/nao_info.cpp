//
// Created by kito on 4/26/18.
//

#include <memory>
#include <boost/asio/ip/host_name.hpp>

#include "libbembelbots/config/botnames.h"
#include "nao_info.h"
#include <framework/logger/logger.h>
#include <framework/util/assert.h>
#include <framework/util/getenv.h>
#include <ipc_sensor_message_generated.h>

#define MAX_ERROR_COUNT 10

NaoInfoResult NaoInfoReader::getNaoInfo(bool docker) {
    NaoInfoResult naoInfoResult;

    LOG_DEBUG << "opening shm with name " << bembelbotsShmName;

    std::unique_ptr<BembelbotsShm> shm;
    try {
        shm = std::make_unique<BembelbotsShm>(bembelbotsShmName, false);
    } catch (const boost::interprocess::interprocess_exception &ex) {
        if (ex.get_error_code() == boost::interprocess::error_code_t::not_found_error) {
            LOG_ERROR << "Backend is not running";
        }
        LOG_FLUSH();
        exit(EXIT_FAILURE);
    }

    bbapi::BembelIpcSensorMessageT data;

    int cnt = 0; // tries
    while ((cnt < MAX_ERROR_COUNT) && !getSensors((*shm)->sensors, data)) {
        ++cnt;
        LOG_WARN << "failed to connect to backend and get data.";
    }

    if (cnt == MAX_ERROR_COUNT) {
        naoInfoResult.error = NaoInfoReaderError::BACKEND_READ_ERROR;
        return naoInfoResult;
    }

    auto naoInfo = &naoInfoResult.naoInfo;
    naoInfo->robotName = data.robotName;

    naoInfo->robotNameStr = DEFS::enum2botname(naoInfo->robotName);

    std::transform(
            naoInfo->robotNameStr.begin(), naoInfo->robotNameStr.end(), naoInfo->robotNameStr.begin(), ::tolower);

    if (docker) {
        std::string name{getEnvVar("ROBOT_NAME")};
        jsassert(!name.empty()) << "set ROBOT_NAME environment variable for docker mode!";
        naoInfo->robotNameStr = name;
    } else if (naoInfo->robotName == RobotName::UNKNOWN) {
        // workaround for playing on unknown robots: use hostname as identifier (same as jrlmonitor)
        naoInfo->robotNameStr = boost::asio::ip::host_name();
    }

    return naoInfoResult;
}

bool NaoInfoReader::getSensors(ipcType &reader, bbapi::BembelIpcSensorMessageT &sensorMsg) {
    bool success = reader.timedProduce(30);

    if (!success) {
        ++failCount;
        return success;
    }

    // unpack flatbuffer
    const auto &sipc{reader.producedData()};
    return sipc.get(sensorMsg);
}

std::string NaoInfo::getName() const {
    return robotNameStr;
}

std::string NaoInfo::getHostname() const {
    return getName();
}

RobotName NaoInfo::getNameId() const {
    return robotName;
}
