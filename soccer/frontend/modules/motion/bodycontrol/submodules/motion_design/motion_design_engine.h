#pragma once

#include <framework/joints/joints.hpp>
#include <bodycontrol/internals/submodule.h>

class MotionDesignBlackboard;

class MotionDesignEngine : public SubModule {
public:
    void connect(rt::Linker &link) override;

    SubModuleReturnValue step(BodyBlackboard *bb) override;

private:
    rt::Context<MotionDesignBlackboard, rt::Write> mdbb;
    BodyBlackboard *bb{nullptr};

    joints::Linear<joints::pos::All> motion;
    joints::Mask updateMask{joints::Mask::All};

    void write_to_blackboard(const joints::Mask &m = joints::Mask::All);
    void set_motion(TimestampMs start, TimestampMs duration);

    void unstiff_chain(const joints::Mask &m);

    joints::pos::All read_from_blackboard();
    joints::stiffness::All set_stiffness();
};
