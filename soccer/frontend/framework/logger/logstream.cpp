#include <string>
#include <string_view>

#include "logstream.h"
#include "xlogger.h"

// LogStream provides the interface for logging
LogStream::LogStream(XLogger *logobj, const int &loglvl, const int &line, const std::string_view &fn, const std::string_view &func)
  : obj(logobj), lm(new LogMessage(loglvl, line, fn, func)) {
}

// Save the streamed data for the logger (during object destruction)
LogStream::~LogStream() {
    obj->log_msg(lm);
}

LogStream &LogStream::operator<<(tEndl) {
    // just append \n
    lm->msg << "\n";
    return *this;
}
