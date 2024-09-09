#pragma once

#include <representations/camera/cam_pose_struct.h>
#include "campose_generated.h"


namespace flatbuffers {
bbapi::CamPose Pack(const ::CamPose &cp);
::CamPose UnPack(const bbapi::CamPose &fcp);
} // namespace flatbuffers
