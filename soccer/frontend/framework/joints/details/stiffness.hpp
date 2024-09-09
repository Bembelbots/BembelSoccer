#pragma once

#include "joints_base.hpp"
#include "operators.hpp"
#include "representations/flatbuffers/types/sensors.h"

#include <representations/flatbuffers/types/actuators.h>

namespace joints {
namespace details {

template<Mask JB>
class Stiffness : public JointsBase<JB> {

public:
    Stiffness() = default;

    explicit Stiffness(const Joints &j) : JointsBase<JB>(j) {}

    explicit Stiffness(const bbipc::JointArray &j) : JointsBase<JB>(j) {}

    explicit Stiffness(const bbipc::Actuators &src) {
        this->fill(0);
        this->read(src);
    }

    explicit Stiffness(const bbipc::Actuators *src) : Stiffness(*src) {}

    void read(const bbipc::Actuators &src) { JointsBase<JB>::read(src.joints.stiffness); }

    void read(const bbipc::Actuators *src) { read(*src); }
    
    void read(const bbipc::JointArray &src) { JointsBase<JB>::read(src); }

    void write(bbipc::Actuators &dst) const { JointsBase<JB>::write(dst.joints.stiffness); }

    void write(bbipc::Actuators *dst) const { write(*dst); }

    void write(bbipc::JointArray &dst) const { JointsBase<JB>::write(dst); }
};
JOINTS_ENABLE_OPERATORS(Stiffness);

} // namespace details
} // namespace joints
