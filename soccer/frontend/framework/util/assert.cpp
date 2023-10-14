#include "assert.h"
#include <boost/filesystem.hpp>
#include <sstream>

std::string jsassertMsg(std::string_view file, int line, std::string_view expr) {
    std::stringstream ss;

    ss << boost::filesystem::path(file.begin(), file.end()).filename().c_str() << ":" << line
       << ": \e[1;31mFAILED ASSERT\e[0m: '" << expr << "'. ";

    return ss.str();
}

std::string jsunreachableMsg(std::string_view file, int line) {
    std::stringstream ss;

    ss << boost::filesystem::path(file.begin(), file.end()).filename().c_str() << ":" << line
       << ": \e[1;31mEXECUTION REACHED UNREACHABLE CALL\e[0m. ";

    return ss.str();
}

void jsassertAbort() {
    LOG_FLUSH();
    std::abort();
}