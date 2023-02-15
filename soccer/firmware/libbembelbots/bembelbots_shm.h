#pragma once

#include "bembelbots.h"
#include "bbcommands.h"

#include <framework/ipc/shared_memory.h>
#include <framework/ipc/tripple_buffer.h>


struct BembelbotsShmContent {
    ipc::TrippleBuffer<BBSensorData> sensors;
    ipc::TrippleBuffer<BBActuatorData> actuators; 
    ipc::TrippleBuffer<BBControlDataBase> control;
};

static const std::string bembelbotsShmName = "bbshm";

using BembelbotsShm = ipc::SharedMemory<BembelbotsShmContent>;

// vim: set ts=4 sw=4 sts=4 expandtab:
