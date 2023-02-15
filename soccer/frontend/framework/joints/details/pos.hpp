#pragma once

#include "actuators.hpp"
#include "joints_base.hpp"
#include "operators.hpp"
#include "sensors.hpp"
#include "tags.hpp"


namespace joints {
namespace details {

template<Mask JB>
class Pos : public JointsBase<JB> {

public:

    Pos() = default;

    explicit Pos(const Joints &j)
        : JointsBase<JB>(j) {
    }

    Pos(FromArrayType, const std::array<float, NR_OF_JOINTS> &init)
        : JointsBase<JB>(init) {
    }

    explicit Pos(const Sensors &src) {
        this->fill(0);
        this->read(src);
    }

    explicit Pos(const Actuators &src) {
        this->fill(0);
        this->read(src);
    }

    void read(const Sensors &src) {
        JointsBase<JB>::read(src.get().data(), angleSensors);
    }

    void read(const Actuators &src) {
        JointsBase<JB>::read(src.get().data(), angleActuators);
    }

    void write(Actuators &dst) const {
        JointsBase<JB>::write(dst.get().data(), angleActuators);
    }

private:

    static const std::array<size_t, NR_OF_JOINTS> angleSensors;
    static const std::array<size_t, NR_OF_JOINTS> angleActuators;
    
};
JOINTS_ENABLE_OPERATORS(Pos);


// Do NOT touch the order of this array. Order of entries must be the same as
// the joint_id enum.
template<Mask JB>
const std::array<size_t, NR_OF_JOINTS> Pos<JB>::angleSensors = {
    headYawPositionSensor,
    headPitchPositionSensor,

    lShoulderPitchPositionSensor,
    lShoulderRollPositionSensor,

    lElbowYawPositionSensor,
    lElbowRollPositionSensor,

    lWristYawPositionSensor,
    lHandPositionSensor,

    lHipYawPitchPositionSensor,

    lHipRollPositionSensor,
    lHipPitchPositionSensor,

    lKneePitchPositionSensor,

    lAnklePitchPositionSensor,
    lAnkleRollPositionSensor,

    rShoulderPitchPositionSensor,
    rShoulderRollPositionSensor,

    rElbowYawPositionSensor,
    rElbowRollPositionSensor,

    rWristYawPositionSensor,
    rHandPositionSensor,

    rHipRollPositionSensor,
    rHipPitchPositionSensor,

    rKneePitchPositionSensor,

    rAnklePitchPositionSensor,
    rAnkleRollPositionSensor,
};

// Do NOT touch the order of this array. Order of entries must be the same as
// the joint_id enum.
template<Mask JB>
const std::array<size_t, NR_OF_JOINTS> Pos<JB>::angleActuators = {
    headYawPositionActuator,
    headPitchPositionActuator,

    lShoulderPitchPositionActuator,
    lShoulderRollPositionActuator,

    lElbowYawPositionActuator,
    lElbowRollPositionActuator,

    lWristYawPositionActuator,
    lHandPositionActuator,

    lHipYawPitchPositionActuator,

    lHipRollPositionActuator,
    lHipPitchPositionActuator,

    lKneePitchPositionActuator,

    lAnklePitchPositionActuator,
    lAnkleRollPositionActuator,

    rShoulderPitchPositionActuator,
    rShoulderRollPositionActuator,

    rElbowYawPositionActuator,
    rElbowRollPositionActuator,

    rWristYawPositionActuator,
    rHandPositionActuator,

    rHipRollPositionActuator,
    rHipPitchPositionActuator,

    rKneePitchPositionActuator,

    rAnklePitchPositionActuator,
    rAnkleRollPositionActuator,
};

} // namespace details
} //namespace joints
