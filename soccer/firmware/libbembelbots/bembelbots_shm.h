#pragma once

#include "ipc_actuator_message_generated.h"
#include "ipc_sensor_message_generated.h"
#include <framework/ipc/shared_memory.h>
#include <framework/ipc/tripple_buffer.h>
#include "flatbuffers_shm.hpp"

static constexpr uint8_t BB_BACKEND_VERSION{4};

struct BembelbotsShmContent {
    ipc::TrippleBuffer<BembelbotsShmFlatbuffer<bbapi::BembelIpcSensorMessage>> sensors;
    ipc::TrippleBuffer<BembelbotsShmFlatbuffer<bbapi::BembelIpcActuatorMessage>> actuators;
};

static const std::string bembelbotsShmName = "bbshm";

using BembelbotsShm = ipc::SharedMemory<BembelbotsShmContent>;

// vim: set ts=4 sw=4 sts=4 expandtab:
