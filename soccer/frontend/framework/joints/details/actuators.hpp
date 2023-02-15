#pragma once

#include "tagged_array.hpp"

#include <libbembelbots/bembelbots.h>


namespace joints {
namespace details {
   
   using Actuators = TaggedArray<
       float, 
       lbbNumOfActuatorIds, 
       struct ActuatorTag>;

} // namespace details
} // namespace joints
