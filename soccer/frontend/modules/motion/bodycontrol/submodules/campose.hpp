#include <bodycontrol/internals/submodule.h>
#include <kinematics/cam_pose.h>

class CalcCamPose : public SubModule {
public:
    SubModuleReturnValue step(BodyBlackboard * bb) override {
        static CameraPose camera_pose;
        auto the_pose = camera_pose.getPose(bb->sensors, bb->bodyAngles, bb->supportFoot);
        bb->bCamPose = the_pose[0];
        bb->tCamPose = the_pose[1];
        return RUNNING;
    }
};




