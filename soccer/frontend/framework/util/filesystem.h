#pragma once

#if V6
#include <filesystem>
namespace fs = std::filesystem;
#else
// gcc7 does not have proper std::filesystem, so use workaround
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;
#endif
