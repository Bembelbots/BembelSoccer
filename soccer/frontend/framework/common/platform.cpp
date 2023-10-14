//#include <behavior/standalone/config.h>
#include "platform.h"

#include <execinfo.h>
#include <cstdlib>

#include <sstream>

static TimestampMs simulationTime = 0;

std::string getTmpDir() {
    char *test = getenv("TMPDIR");
    if (!test) {
        return "/tmp/";
    }
    return std::string(test).append("/");
}

std::string getBacktrace() {
    std::ostringstream ostr;
    const int maxsz = 20;

    void *array[maxsz];
    char **strings;

    int size = backtrace(array, maxsz);
    strings = backtrace_symbols(array, size);

    ostr << "Obtained " << size << " stack frames.\n";

    for (int i = 0; i < size; i++) {
        ostr << strings[i] << "\n";
    }

    free(strings);

    return ostr.str();
}

// vim: set ts=4 sw=4 sts=4 expandtab:
