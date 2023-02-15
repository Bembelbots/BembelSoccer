#pragma once

#include "tagged_array.hpp"

#include <libbembelbots/bembelbots.h>


namespace joints {
namespace details {
   
   using Sensors = TaggedArray<float, lbbNumOfSensorIds, struct SensorTag>;

} // namespace details
} // namespace joints
