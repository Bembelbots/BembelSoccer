#include <sys/prctl.h>

#include "util.h"
#include "../logger/logger.h"


void set_current_thread_name(const std::string_view name) {
    if (0 != prctl(PR_SET_NAME, name.data()))
        LOG_ERROR << "Failed to set thread name: " << name;
}
