
#include "bodycontrol/internals/submodule.h"
#include <bodycontrol/utils/stiffness_control.hpp>
#include <shared/backend/bembelbots.h>

class LiveTest : public SubModule {
public:
    LiveTest(){
        foo =0.0;
    }
    Joints<HeadYaw> one;
    Joints<HeadYaw> two;
    JointInterpolation<HeadYaw> inter;
    int start_time;
public:

    bool state_machine(BodyBlackboard * bb) override {
        initial_state(to_one, true){
            float diff = fabs((Joints<HeadYaw>().readActuatorAngles(bb->actuators) - one).values[0]);
            state_initialization{
                inter = JointInterpolation<HeadYaw>(two, one, 2000.0f);
                start_time = bb->timestamp_ms;
                LOG_INFO << "one" << EL;
            }
            inter.getJoints(bb->timestamp_ms - start_time).writeActuatorAngles(bb->actuators);
            if (diff < 1.0f/200.0f) goto to_two;
        }
        state(to_two, true){
            float diff = fabs((Joints<HeadYaw>().readActuatorAngles(bb->actuators) - two).values[0]);
            state_initialization{
                inter = JointInterpolation<HeadYaw>(one, two, 2000.0f);
                start_time = bb->timestamp_ms;
                LOG_INFO << "two" << EL;
            }
            inter.getJoints(bb->timestamp_ms - start_time).writeActuatorAngles(bb->actuators);
            if (diff < 1.0f/200.0f) goto to_one;
        }
    }

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        body_stance.readSensorAngles(bb->sensors);
        body_stance.writeActuatorAngles(bb->actuators);
        setStiffness(0.8, bb);
        run_state_machine(bb);
        return MOTION_STABLE;
    }

    void reset() override {
        one.values[0] = -2.0f;
        two.values[0] = 2.0f;
    }
private:
    float foo;
    Joints<Body> body_stance;
};




