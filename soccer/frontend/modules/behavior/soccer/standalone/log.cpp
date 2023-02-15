#include "config.h"
#include "log.h"


using namespace stab;


Log &Log::instance() {
    static Log log;

    return log;
}

Log::Log() : BaseLoggerBackend("", "stablogger") {}

void Log::initialize() {
    if (XLogger::log_map.find(LOGID) != XLogger::log_map.end()) {
        return;
    }

    XLogger *xlog = new XLogger(LOGID);

    xlog->add_backend(this, "[%%FANCYLVL%%] %%MSG%%\n");

    xlog->set_loglvl_desc(10, "ERR");
    xlog->set_loglvl_desc(7, "W");
    xlog->set_loglvl_desc(5, "i");
    xlog->set_loglvl_desc(3, "DEBUG");
    xlog->set_loglvl_desc(1, "PROFILE");

    xlog->set_min_loglvl(1);
    xlog->set_msg_stripping(false);


    XLogger *xlogsay = new XLogger(LOGSAYID);

    xlogsay->add_backend(this, "[%%FANCYLVL%%] %%MSG%%\n");

    xlogsay->set_loglvl_desc(10, "ERR");
    xlogsay->set_loglvl_desc(7, "W");
    xlogsay->set_loglvl_desc(5, "i");
    xlogsay->set_loglvl_desc(3, "DEBUG");
    xlogsay->set_loglvl_desc(1, "PROFILE");

    xlogsay->set_min_loglvl(1);
    xlogsay->set_msg_stripping(false);
}

void Log::write(const std::string &msg) {
    ss << "[" << robot << "]" << msg;
}

std::string Log::flush() {
    std::string msgs = ss.str();
    ss.str(""); // clear content of string stream
    return msgs;
}


// vim: set ts=4 sw=4 sts=4 expandtab:

