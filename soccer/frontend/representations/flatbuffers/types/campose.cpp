#include "campose.h"
#include "cam_pose_message_generated.h"
#include "representations/camera/cam_pose_struct.h"

namespace flatbuffers {
bbapi::CamPose Pack(const ::CamPose &cp) {
    return bbapi::CamPose((float[]){cp.v.x(), cp.v.y(), cp.v.z()}, (float[]){cp.r.x(), cp.r.y(), cp.r.z()});
}

::CamPose UnPack(const bbapi::CamPose &fcp) {
    ::CamPose cp;
    const auto &v{*fcp.v()};
    cp.v << v[0], v[1], v[2];

    const auto &r{*fcp.r()};
    cp.r << r[0], r[1], r[2];

    return cp;
}
} // namespace flatbuffers
