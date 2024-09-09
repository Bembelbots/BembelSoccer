#pragma once

#include "joints_base.hpp"
#include "operators.hpp"


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

    explicit Current(const bbipc::Sensors &src) {
        this->fill(0);
        this->read(src); 
    }

    void read(const bbipc::Sensors &src) {
        JointsBase<JB>::read(src.joints.current);
    }
};
JOINTS_ENABLE_OPERATORS(Current);
    
} // namespace details
} // namespace joints
