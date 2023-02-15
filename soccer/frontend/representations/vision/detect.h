#pragma once
#include "visiondefinitions.h"
#include <optional>

using VisionResultVec = std::vector<VisionResult>;

struct VisionBall {
    int id;
    std::optional<VisionResult> vr;
};