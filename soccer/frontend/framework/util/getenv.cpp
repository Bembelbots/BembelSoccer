#include "getenv.h"
#include <cstdlib>

std::string getEnvVar(const std::string &key) {
    char *val = getenv(key.c_str());
    return (val == nullptr) ? "" : std::string(val);
}
