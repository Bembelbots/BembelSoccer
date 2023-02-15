#pragma once

#include "joints_base.hpp"
#include "operators.hpp"
#include "sensors.hpp"
#include "tags.hpp"


namespace joints {
namespace details {
    
template<Mask JB>
class Current : public JointsBase<JB> {

public:

    Current()
        : JointsBase<JB>() {
    }


    explicit Current(const Joints &j)
        : JointsBase<JB>(j) {
    }

    Current(FromArrayType, const std::array<float, NR_OF_JOINTS> &init)
        : JointsBase<JB>(init) {
    }

    explicit Current(const Sensors &src) {
        this->fill(0);
        this->read(src); 
    }

    void read(const Sensors &src) {
        JointsBase<JB>::read(src.get().data(), currentSensors);
    }

private:

    static const std::array<size_t, NR_OF_JOINTS> currentSensors;

};
JOINTS_ENABLE_OPERATORS(Current);


// Do NOT touch the order of this array. Order of entries must be the same as
// the joint_id enum.
template<Mask JB>
const std::array<size_t, NR_OF_JOINTS> Current<JB>::currentSensors = {
    headYawCurrentSensor,
    headPitchCurrentSensor,

    lShoulderPitchCurrentSensor,
    lShoulderRollCurrentSensor,

    lElbowYawCurrentSensor,
    lElbowRollCurrentSensor,

    lWristYawCurrentSensor,
    lHandCurrentSensor,

    lHipYawPitchCurrentSensor,

    lHipRollCurrentSensor,
    lHipPitchCurrentSensor,

    lKneePitchCurrentSensor,

    lAnklePitchCurrentSensor,
    lAnkleRollCurrentSensor,

    rShoulderPitchCurrentSensor,
    rShoulderRollCurrentSensor,

    rElbowYawCurrentSensor,
    rElbowRollCurrentSensor,

    rWristYawCurrentSensor,
    rHandCurrentSensor,

    rHipRollCurrentSensor,
    rHipPitchCurrentSensor,

    rKneePitchCurrentSensor,

    rAnklePitchCurrentSensor,
    rAnkleRollCurrentSensor,
};
    
} // namespace details
} // namespace joints
