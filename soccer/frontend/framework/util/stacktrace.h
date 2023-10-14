#pragma once

#include "../logger/logger.h"

#include <execinfo.h>
#include <cxxabi.h>

/**
 * Print a pretty stack trace
 * Source: https://oroboro.com/stack-trace-on-crash/
 */
inline void printStackTrace(unsigned int max_frames = 63) {
    LOG_ERROR << "Stacktrace:";

    // storage array for stack trace address data
    void *addrlist[max_frames + 1];

    // retrieve current stack addresses
    unsigned int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void *));

    if (addrlen == 0) {
        LOG_ERROR << "Could not retreive a stacktrace";
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // Actually it will be ## program address function + offset
    // this array must be free()-ed
    char **symbollist = backtrace_symbols(addrlist, addrlen);

    size_t funcnamesize = 1024;
    char funcname[1024];

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (unsigned int i = 4; i < addrlen; i++) {
        char *begin_name   = NULL;
        char *begin_offset = NULL;
        char *end_offset   = NULL;

        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p) {
            if (*p == '(') {
                begin_name = p;
            } else if (*p == '+') {
                begin_offset = p;
            } else if (*p == ')' && (begin_offset || begin_name)) {
                end_offset = p;
            }
        }

        if (begin_name && end_offset && (begin_name < end_offset)) {
            *begin_name++   = '\0';
            *end_offset++   = '\0';
            if (begin_offset) {
                *begin_offset++ = '\0';
            }

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status = 0;
            char *ret = abi::__cxa_demangle(begin_name, funcname,
                                            &funcnamesize, &status);
            char *fname = begin_name;
            if (status == 0) {
                fname = ret;
            }

            if (begin_offset) {
                LOG_ERROR << " " << symbollist[i] << " ( " << fname << " + "
                          << begin_offset << ") " << end_offset;
            } else {
                LOG_ERROR << " " << symbollist[i] << " ( " << fname << " ) "
                          << end_offset;
            }
        } else {
            // couldn't parse the line? print the whole line.
            LOG_ERROR << "  " << symbollist[i];
        }
    }

    free(symbollist);

    LOG_FLUSH();
}

// vim: set ts=4 sw=4 sts=4 expandtab:
