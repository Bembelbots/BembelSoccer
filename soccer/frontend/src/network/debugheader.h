#pragma once

#include <cstdint>
#include <string_view>

#include <representations/bembelbots/types.h>

constexpr char DEBUG_IMAGE_MAGIC[8]{"BBimage"};
constexpr uint8_t DEBUG_IMAGE_VERSION{2};

struct DebugImageHeader {
    char magic[8];

    uint8_t version{1};   // struct version, increase when layout is changed
    uint8_t camera{0};
    ImageCodec codec{ImageCodec::JPG};
    
    uint32_t timestamp{0};
    
    uint64_t imageSize{0};
    uint64_t vrSize{0};
};
