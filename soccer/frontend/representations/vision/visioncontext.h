#pragma once
#include <memory>

class HTWKVisionContext;

struct VisionContext {
    std::shared_ptr<HTWKVisionContext> htwk;
    std::string configPath;
};