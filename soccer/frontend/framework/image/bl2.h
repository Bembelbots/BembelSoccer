#pragma once

#include <cstddef>
#include <string>

class CamImage;

class Bl2 {
public:
    static CamImage load(const std::string &filename, size_t w, size_t h, int camera);
};
