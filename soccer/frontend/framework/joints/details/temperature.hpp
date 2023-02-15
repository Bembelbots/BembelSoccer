#pragma once

#include "joints_base.hpp"
#include "operators.hpp"
#include "sensors.hpp"
#include "tags.hpp"


namespace joints {
namespace details {
    
template<Mask JB>
class Temperature : public JointsBase<JB> {

public:

    explicit Temperature(const Joints &j)
        : JointsBase<JB>(j) {
    }

    Temperature(FromArrayType, const std::array<float, NR_OF_JOINTS> &init)
        : JointsBase<JB>(init) {
    }

    explicit Temperature(const Sensors &src) {
        this->fill(0);
        this->read(src); 
    }

    void read(const Sensors &src) {
        JointsBase<JB>::read(src.get().data(), temperatureSensors);
    }

private:

    static const std::array<size_t, NR_OF_JOINTS> temperatureSensors;

};
JOINTS_ENABLE_OPERATORS(Temperature);


// Do NOT touch the order of this array. Order of entries must be the same as
// the joint_id enum.
template<Mask JB>
const std::array<size_t, NR_OF_JOINTS> Temperature<JB>::temperatureSensors = {
    headYawTemperatureSensor,
    headPitchTemperatureSensor,

    lShoulderPitchTemperatureSensor,
    lShoulderRollTemperatureSensor,

    lElbowYawTemperatureSensor,
    lElbowRollTemperatureSensor,

    lWristYawTemperatureSensor,
    lHandTemperatureSensor,

    lHipYawPitchTemperatureSensor,

    lHipRollTemperatureSensor,
    lHipPitchTemperatureSensor,

    lKneePitchTemperatureSensor,

    lAnklePitchTemperatureSensor,
    lAnkleRollTemperatureSensor,

    rShoulderPitchTemperatureSensor,
    rShoulderRollTemperatureSensor,

    rElbowYawTemperatureSensor,
    rElbowRollTemperatureSensor,

    rWristYawTemperatureSensor,
    rHandTemperatureSensor,

    rHipRollTemperatureSensor,
    rHipPitchTemperatureSensor,

    rKneePitchTemperatureSensor,

    rAnklePitchTemperatureSensor,
    rAnkleRollTemperatureSensor,
};
    
} // namespace details
} // namespace joints
