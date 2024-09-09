#pragma once

#include "framework/joints/body_v6.h"
#include "joints_base.hpp"
#include "operators.hpp"
#include "representations/flatbuffers/types/sensors.h"

#include <optional>
#include <representations/flatbuffers/types/actuators.h>

namespace joints {
namespace details {

template<Mask JB>
class Pos : public JointsBase<JB> {

public:
    Pos() = default;

    explicit Pos(const Joints &j) : JointsBase<JB>(j) {}

    explicit Pos(const bbipc::JointArray &j) : JointsBase<JB>(j) {}

    explicit Pos(const bbipc::Sensors &src) {
        this->fill(0);
        this->read(src);
    }

    explicit Pos(const bbipc::Actuators &src) {
        this->fill(0);
        this->read(src);
    }

    explicit Pos(const bbipc::Actuators *src) : Pos(*src) {}

    void read(const bbipc::Sensors &src) { JointsBase<JB>::read(src.joints.position); }

    void read(const bbipc::Actuators &src) { JointsBase<JB>::read(src.joints.position); }

    void read(const bbipc::Actuators *src) { read(*src); }

    void read(const bbipc::JointArray &src) { JointsBase<JB>::read(src); }

    void write(bbipc::Actuators &dst) const { JointsBase<JB>::write(dst.joints.position); }

    void write(bbipc::Actuators *dst) const { write(*dst); }

    void write(bbipc::JointArray &dst) const { JointsBase<JB>::write(dst); }

    std::optional<std::vector<JointNames>> isInvalid() const {
        constexpr auto &limits{joints::CONSTRAINTS};
        std::vector<JointNames> err;

        this->each([&](const JointNames &i) {
            const auto &limit{limits.at(i)};
            if (this->at(i) < limit.min)
                err.emplace_back(i);
            else if (this->at(i) > limit.max)
                err.emplace_back(i);
            else if (std::isinf(this->at(i)))
                err.emplace_back(i);
            else if (std::isnan(this->at(i)))
                err.emplace_back(i);
        });

        if (err.empty())
            return std::nullopt;

        return err;
    }
    static constexpr float ERROR_THRESHOLD{0.0000001f};
};
JOINTS_ENABLE_OPERATORS(Pos);

} // namespace details
} //namespace joints
