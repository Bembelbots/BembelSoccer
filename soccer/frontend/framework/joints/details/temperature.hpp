#pragma once

#include "joints_base.hpp"
#include "operators.hpp"

#include <representations/flatbuffers/types/sensors.h>

namespace joints {
namespace details {
    
template<Mask JB>
class Temperature : public JointsBase<JB> {

public:

    explicit Temperature(const Joints &j)
        : JointsBase<JB>(j) {
    }

    explicit Temperature(const bbipc::Sensors &src) {
        this->fill(0);
        this->read(src); 
    }

    void read(const bbipc::Sensors &src) {
        JointsBase<JB>::read(src.joints.temperature);
    }
};
JOINTS_ENABLE_OPERATORS(Temperature);

} // namespace details
} // namespace joints
