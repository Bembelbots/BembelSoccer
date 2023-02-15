#pragma once

/**
 *
 */

#if BB_BENCHMARKING
# include "timer.h"
# include <framework/logger/logger.h>
#else
# include <representations/bembelbots/constants.h> // import NOOP statement
#endif

#if BB_BENCHMARKING
# define START_TIMER(name) Timer::start(name)
#else
# define START_TIMER(name) NOOP
#endif

#if BB_BENCHMARKING
# define STOP_TIMER Timer::stop()
#else
# define STOP_TIMER NOOP
#endif

#if BB_BENCHMARKING
# define LAST_MEASSUREMENT(name) Timer::getExistingTimer(name).lastMeassurement()
#endif

#if BB_BENCHMARKING
# define MEASSUREMENTS_AS_STR Timer::str()
#endif

#if BB_BENCHMARKING
# define PRINT_MEASSUREMENTS \
    LOG_INFO << "** Meassurements **"; \
    LOG_INFO << MEASSUREMENTS_AS_STR
#else
# define PRINT_MEASSUREMENTS NOOP
#endif

#if BB_BENCHMARKING
# define SAVE_MEASSUREMENTS_AS_CSV(fname) Timer::saveAsCsv(fname)
#else
# define SAVE_MEASSUREMENTS_AS_CSV(fname) NOOP
#endif

// vim: set ts=4 sw=4 sts=4 expandtab:
