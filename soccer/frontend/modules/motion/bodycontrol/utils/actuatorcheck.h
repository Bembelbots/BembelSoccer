#pragma once

namespace bbipc {
class Sensors;
class Actuators;
} // namespace bbipc

bool checkAndCorrectActuators(bbipc::Actuators *actuators, const bbipc::Sensors &sensors);
