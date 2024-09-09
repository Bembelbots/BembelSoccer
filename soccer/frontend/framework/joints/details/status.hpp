#pragma once

#include "joints_base.hpp"
#include "operators.hpp"

#include <representations/flatbuffers/types/sensors.h>


namespace joints {
namespace details {
    
template<Mask JB>
class Status : public JointsBase<JB> {

public:

    Status()
        : JointsBase<JB>() {
    }

    explicit Status(const Joints &j)
        : JointsBase<JB>(j) {
    }

    explicit Status(const bbipc::Sensors &src) {
        this->fill(0);
        this->read(src); 
    }

    void read(const bbipc::Sensors &src) {
        JointsBase<JB>::read(src.joints.status);
    }
};
JOINTS_ENABLE_OPERATORS(Status);

} // namespace details
} // namespace joints
