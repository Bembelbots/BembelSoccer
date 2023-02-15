#pragma once

#include "joints_base.hpp"
#include "operators.hpp"
#include "tags.hpp"


namespace joints {
namespace details {
    
template<Mask JB>
class Stiffness : public JointsBase<JB> {

public:

    Stiffness() = default;

    explicit Stiffness(const Joints &j)
        : JointsBase<JB>(j) {
    }

    Stiffness(FromArrayType, const std::array<float, NR_OF_JOINTS> &init)
        : JointsBase<JB>(init) {
    }

    explicit Stiffness(const Actuators &src) {
        this->fill(0);
        this->read(src);
    }

    void read(const Actuators &src) {
        JointsBase<JB>::read(src.get().data(), stiffnessActuators);
    }

    void write(Actuators &dst) const {
        JointsBase<JB>::write(dst.get().data(), stiffnessActuators);
    }

private:

    static const std::array<size_t, NR_OF_JOINTS> stiffnessActuators;
    
};
JOINTS_ENABLE_OPERATORS(Stiffness);


// Do NOT touch the order of this array. Order of entries must be the same as
// the joint_id enum.
template<Mask JB>
const std::array<size_t, NR_OF_JOINTS> Stiffness<JB>::stiffnessActuators = {
    headYawHardnessActuator,
    headPitchHardnessActuator,

    lShoulderPitchHardnessActuator,
    lShoulderRollHardnessActuator,

    lElbowYawHardnessActuator,
    lElbowRollHardnessActuator,

    lWristYawHardnessActuator,
    lHandHardnessActuator,

    lHipYawPitchHardnessActuator,

    lHipRollHardnessActuator,
    lHipPitchHardnessActuator,

    lKneePitchHardnessActuator,

    lAnklePitchHardnessActuator,
    lAnkleRollHardnessActuator,

    rShoulderPitchHardnessActuator,
    rShoulderRollHardnessActuator,

    rElbowYawHardnessActuator,
    rElbowRollHardnessActuator,

    rWristYawHardnessActuator,
    rHandHardnessActuator,

    rHipRollHardnessActuator,
    rHipPitchHardnessActuator,

    rKneePitchHardnessActuator,

    rAnklePitchHardnessActuator,
    rAnkleRollHardnessActuator,
};

} // namespace details
} // namespace joints
