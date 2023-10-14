#pragma once

#ifndef LOGID
#define LOGID "system"
#endif

#ifndef LOGSAYID
#define LOGSAYID "logsay"
#endif

#ifndef LOGDATAID
#define LOGDATAID "data"
#endif

#define LOG_LVL_MAX 1
#define LOG_LVL_MORE 2
#define LOG_LVL_DEBUG 3
#define LOG_LVL_INFO 5
#define LOG_LVL_WARN 7
#define LOG_LVL_ERROR 10

// by passing -DXLOG_FORCE_MIN_LVL=n to the compiler,
// one may force any log level
#ifdef XLOG_FORCE_MIN_LVL
#define XLOG_MIN_LOG_LVL XLOG_FORCE_MIN_LVL
#else
#if BUILDING_RELEASE
#define XLOG_MIN_LOG_LVL LOG_LVL_INFO
#else
#define XLOG_MIN_LOG_LVL LOG_LVL_DEBUG
#endif // log-lvl by NDEBUG endif
#endif // force log-lvl endif

// vim: set ts=4 sw=4 sts=4 expandtab:
