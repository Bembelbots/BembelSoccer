#pragma once

#include "joints_base.hpp"
#include "operators.hpp"
#include "sensors.hpp"
#include "tags.hpp"


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

    Status(FromArrayType, const std::array<float, NR_OF_JOINTS> &init)
        : JointsBase<JB>(init) {
    }

    explicit Status(const Sensors &src) {
        this->fill(0);
        this->read(src); 
    }

    void read(const Sensors &src) {
        JointsBase<JB>::read(src.get().data(), statusSensors);
    }

private:

    static const std::array<size_t, NR_OF_JOINTS> statusSensors;

};
JOINTS_ENABLE_OPERATORS(Status);


// Do NOT touch the order of this array. Order of entries must be the same as
// the joint_id enum.
template<Mask JB>
const std::array<size_t, NR_OF_JOINTS> Status<JB>::statusSensors = {
    headYawStatusSensor,
    headPitchStatusSensor,

    lShoulderPitchStatusSensor,
    lShoulderRollStatusSensor,

    lElbowYawStatusSensor,
    lElbowRollStatusSensor,

    lWristYawStatusSensor,
    lHandStatusSensor,

    lHipYawPitchStatusSensor,

    lHipRollStatusSensor,
    lHipPitchStatusSensor,

    lKneePitchStatusSensor,

    lAnklePitchStatusSensor,
    lAnkleRollStatusSensor,

    rShoulderPitchStatusSensor,
    rShoulderRollStatusSensor,

    rElbowYawStatusSensor,
    rElbowRollStatusSensor,

    rWristYawStatusSensor,
    rHandStatusSensor,

    rHipRollStatusSensor,
    rHipPitchStatusSensor,

    rKneePitchStatusSensor,

    rAnklePitchStatusSensor,
    rAnkleRollStatusSensor,
};
    
} // namespace details
} // namespace joints
