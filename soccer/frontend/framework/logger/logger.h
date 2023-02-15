#pragma once

#include "loglvl.h"
#include "xlogger.h"
#include "logstream.h"

// set this before you include logger.h
#ifndef XLOG_MIN_LOG_LVL
# error "XLOG_MIN_LOG_LVL has not been defined"
#endif

// This is _the_ mAgIc MaIn #DEFINE - do not change!
#define BBLOG(lvl, LOGID, cond)   \
    if ((cond) && XLogger::check_loglevel(lvl, LOGID))    \
        LogStream(XLogger::get(LOGID), lvl, __LINE__, __FILE__, __FUNCTION__)

#define CONCAT(a, b) a ## b
#define BUILD_VARNAME(name, line) CONCAT(name, line)
#define MAGIC_COUNTER BUILD_VARNAME(logCounter_, __LINE__)

#define BBLOG_EVERY_N(lvl, LOGID, n) \
    static int MAGIC_COUNTER = 0; \
    MAGIC_COUNTER += 1 - (MAGIC_COUNTER >= n) * n; \
    BBLOG(lvl, LOGID, (MAGIC_COUNTER == 1))

#define BBLOG_IF_EVERY_N(lvl, LOGID, cond, n) \
    static int MAGIC_COUNTER = 0; \
    MAGIC_COUNTER += static_cast<bool>(cond) - (MAGIC_COUNTER >= n) * n; \
    BBLOG(lvl, LOGID, (cond && MAGIC_COUNTER == 1))

#define BBLOG_FIRST_N(lvl, LOGID, n) \
    static int MAGIC_COUNTER = 0; \
    MAGIC_COUNTER += (MAGIC_COUNTER <= n); \
    BBLOG(lvl, LOGID, (MAGIC_COUNTER <= n))

// FakeLogStream as compiletime-replacement
#define FAKELOG() FakeLogStream()

// data logger stuff....
#define RAWDATALOG(lvl, log_id) \
    LogStream(XLogger::get(log_id), lvl, __LINE__, __FILE__, __FUNCTION__)
#define LOG_DATA RAWDATALOG(10, LOGDATAID)

#define WARN_NOT_IMPLEMENTED \
    LOG_WARN << __PRETTY_FUNCTION__ << " not implemented!";

// block until all queued messages are written (used for stacktrace)
#define LOG_FLUSH() XLogger::get(LOGID)->flush()

// Some ready-to-use examples for special loggers
// to be used like this:
// INFO_LOG << "my message goes here" << "another one" << 123";
// using std::endl is okay, further formatting is not (yet) implemented

#if XLOG_MIN_LOG_LVL <= LOG_LVL_MAX
# define LOG_MAX                     BBLOG(LOG_LVL_MAX, LOGID, true)
# define LOG_MAX_IF(cond)            BBLOG(LOG_LVL_MAX, LOGID, cond)
# define LOG_MAX_EVERY_N(n)          BBLOG_EVERY_N(LOG_LVL_MAX, LOGID, n)
# define LOG_MAX_IF_EVERY_N(cond, n) BBLOG_IF_EVERY_N(LOG_LVL_MAX, LOGID, cond, n)
# define LOG_MAX_FIRST_N(n)          BBLOG_FIRST_N(LOG_LVL_MAX, LOGID, n)
# define IS_MAX_LOG true && XLogger::check_loglevel(1, LOGID)
#else
# define LOG_MAX                     if(false) FAKELOG()
# define LOG_MAX_IF(cond)            if(false) FAKELOG()
# define LOG_MAX_EVERY_N(n)          if(false) FAKELOG()
# define LOG_MAX_IF_EVERY_N(cond, n) if(false) FAKELOG()
# define LOG_MAX_FIRST_N(n)          if(false) FAKELOG()
# define IS_MAX_LOG false
#endif

#if XLOG_MIN_LOG_LVL <= LOG_LVL_MORE
# define LOG_MORE                     BBLOG(LOG_LVL_MORE, LOGID, true)
# define LOG_MORE_IF(cond)            BBLOG(LOG_LVL_MORE, LOGID, cond)
# define LOG_MORE_EVERY_N(n)          BBLOG_EVERY_N(LOG_LVL_MORE, LOGID, n)
# define LOG_MORE_IF_EVERY_N(cond, n) BBLOG_IF_EVERY_N(LOG_LVL_MORE, LOGID, cond, n)
# define LOG_MORE_FIRST_N(n)          BBLOG_FIRST_N(LOG_LVL_MORE, LOGID, n)
# define IS_MORE_DEBUG true && XLogger::check_loglevel(LOG_LVL_MORE, LOGID)
#else
# define LOG_MORE                     if(false) FAKELOG()
# define LOG_MORE_IF(cond)            if(false) FAKELOG()
# define LOG_MORE_EVERY_N(n)          if(false) FAKELOG()
# define LOG_MORE_IF_EVERY_N(cond, n) if(false) FAKELOG()
# define LOG_MORE_FIRST_N(n)          if(false) FAKELOG()
# define IS_MORE_DEBUG false
#endif

#if XLOG_MIN_LOG_LVL <= LOG_LVL_DEBUG
# define LOG_DEBUG                     BBLOG(LOG_LVL_DEBUG, LOGID, true)
# define LOG_DEBUG_IF(cond)            BBLOG(LOG_LVL_DEBUG, LOGID, cond)
# define LOG_DEBUG_EVERY_N(n)          BBLOG_EVERY_N(LOG_LVL_DEBUG, LOGID, n)
# define LOG_DEBUG_IF_EVERY_N(cond, n) BBLOG_IF_EVERY_N(LOG_LVL_DEBUG, LOGID, cond, n)
# define LOG_DEBUG_FIRST_N(n)          BBLOG_FIRST_N(LOG_LVL_DEBUG, LOGID, n)

# define LOG_SAY_DEBUG                     BBLOG(LOG_LVL_INFO, LOGSAYID, true)
# define LOG_SAY_DEBUG_IF(cond)            BBLOG(LOG_LVL_INFO, LOGSAYID, cond)
# define LOG_SAY_DEBUG_EVERY_N(n)          BBLOG_EVERY_N(LOG_LVL_INFO, LOGSAYID, n)
# define LOG_SAY_DEBUG_IF_EVERY_N(cond, n) BBLOG_IF_EVERY_N(LOG_LVL_INFO, LOGSAYID, cond, n)
# define LOG_SAY_DEBUG_FIRST_N(n)          BBLOG_FIRST_N(LOG_LVL_INFO, LOGSAYID, n)

# define IS_DEBUG true && XLogger::check_loglevel(LOG_LVL_DEBUG, LOGID) \
                    && XLogger::check_loglevel(LOG_LVL_DEBUG, LOGSAYID)
#else
# define LOG_DEBUG                     if(false) FAKELOG()
# define LOG_DEBUG_IF(cond)            if(false) FAKELOG()
# define LOG_DEBUG_EVERY_N(n)          if(false) FAKELOG()
# define LOG_DEBUG_IF_EVERY_N(cond, n) if(false) FAKELOG()
# define LOG_DEBUG_FIRST_N(n)          if(false) FAKELOG()

# define LOG_SAY_DEBUG                     if(false) FAKELOG()
# define LOG_SAY_DEBUG_IF(cond)            if(false) FAKELOG()
# define LOG_SAY_DEBUG_EVERY_N(n)          if(false) FAKELOG()
# define LOG_SAY_DEBUG_IF_EVERY_N(cond, n) if(false) FAKELOG()
# define LOG_SAY_DEBUG_FIRST_N(n)          if(false) FAKELOG()

# define IS_DEBUG false
#endif

#if XLOG_MIN_LOG_LVL <= LOG_LVL_INFO
# define LOG_INFO                     BBLOG(LOG_LVL_INFO, LOGID, true)
# define LOG_INFO_IF(cond)            BBLOG(LOG_LVL_INFO, LOGID, cond)
# define LOG_INFO_EVERY_N(n)          BBLOG_EVERY_N(LOG_LVL_INFO, LOGID, n)
# define LOG_INFO_IF_EVERY_N(cond, n) BBLOG_IF_EVERY_N(LOG_LVL_INFO, LOGID, cond, n)
# define LOG_INFO_FIRST_N(n)          BBLOG_FIRST_N(LOG_LVL_INFO, LOGID, n)

# define LOG_SAY                     BBLOG(LOG_LVL_INFO, LOGSAYID, true)
# define LOG_SAY_IF(cond)            BBLOG(LOG_LVL_INFO, LOGSAYID, cond)
# define LOG_SAY_EVERY_N(n)          BBLOG_EVERY_N(LOG_LVL_INFO, LOGSAYID, n)
# define LOG_SAY_IF_EVERY_N(cond, n) BBLOG_IF_EVERY_N(LOG_LVL_INFO, LOGSAYID, cond, n)
# define LOG_SAY_FIRST_N(n)          BBLOG_FIRST_N(LOG_LVL_INFO, LOGSAYID, n)

# define IS_INFO true && XLogger::check_loglevel(LOG_LVL_INFO, LOGID) \
                && XLogger::check_loglevel(LOG_LVL_INFO, LOGSAYID)
#else
# define LOG_INFO                     if(false) FAKELOG()
# define LOG_INFO_IF(cond)            if(false) FAKELOG()
# define LOG_INFO_EVERY_N(n)          if(false) FAKELOG()
# define LOG_INFO_IF_EVERY_N(cond, n) if(false) FAKELOG()
# define LOG_INFO_FIRST_N(n)          if(false) FAKELOG()

# define LOG_SAY                     if(false) FAKELOG()
# define LOG_SAY_IF(cond)            if(false) FAKELOG()
# define LOG_SAY_EVERY_N(n)          if(false) FAKELOG()
# define LOG_SAY_IF_EVERY_N(cond, n) if(false) FAKELOG()
# define LOG_SAY_FIRST_N(n)          if(false) FAKELOG()

# define IS_INFO false
#endif

#if XLOG_MIN_LOG_LVL <= LOG_LVL_WARN
# define LOG_WARN                     BBLOG(LOG_LVL_WARN, LOGID, true)
# define LOG_WARN_IF(cond)            BBLOG(LOG_LVL_WARN, LOGID, cond)
# define LOG_WARN_EVERY_N(n)          BBLOG_EVERY_N(LOG_LVL_WARN, LOGID, n)
# define LOG_WARN_IF_EVERY_N(cond, n) BBLOG_IF_EVERY_N(LOG_LVL_WARN, LOGID, cond, n)
# define LOG_WARN_FIRST_N(n)          BBLOG_FIRST_N(LOG_LVL_WARN, LOGID, n)
# define IS_WARN true && XLogger::check_loglevel(LOG_LVL_WARN, LOGID)
#else
# define LOG_WARN                     if(false) FAKELOG()
# define LOG_WARN_IF(cond)            if(false) FAKELOG()
# define LOG_WARN_EVERY_N(n)          if(false) FAKELOG()
# define LOG_WARN_IF_EVERY_N(cond, n) if(false) FAKELOG()
# define LOG_WARN_FIRST_N(n)          if(false) FAKELOG()
#define IS_WARN false
#endif

#if XLOG_MIN_LOG_LVL <= LOG_LVL_ERROR
# define LOG_ERROR                     BBLOG(LOG_LVL_ERROR, LOGID, true)
# define LOG_ERROR_IF(cond)            BBLOG(LOG_LVL_ERROR, LOGID, cond)
# define LOG_ERROR_EVERY_N(n)          BBLOG_EVERY_N(LOG_LVL_ERROR, LOGID, n)
# define LOG_ERROR_IF_EVERY_N(cond, n) BBLOG_IF_EVERY_N(LOG_LVL_ERROR, LOGID, cond, n)
# define LOG_ERROR_FIRST_N(n)          BBLOG_FIRST_N(LOG_LVL_ERROR, LOGID, n)
# define IS_ERROR true && XLogger::check_loglevel(LOG_LVL_ERROR, LOGID)
#else
# define LOG_ERROR                     if(false) FAKELOG()
# define LOG_ERROR_IF(cond)            if(false) FAKELOG()
# define LOG_ERROR_EVERY_N(n)          if(false) FAKELOG()
# define LOG_ERROR_IF_EVERY_N(cond, n) if(false) FAKELOG()
# define LOG_ERROR_FIRST_N(n)          if(false) FAKELOG()
# define IS_ERROR false
#endif
