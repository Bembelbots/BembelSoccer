#include <atomic>
#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>
#include <functional>
#include <string_view>

#include <boost/lockfree/queue.hpp>

#include "xlogger.h"
#include "backends.h"

#include "../common/platform.h"
#include "../util/converter.h"
#include "../common/macros.h"
#include "../thread/threadmanager.h"

using std::this_thread::sleep_for;
using namespace std::chrono_literals;

XLogger::tXLoggerMap XLogger::log_map;

// IO work class
// wraps boost::lockfree, so it does not need to be included in xlogger.h
class LogIO {
public:
    boost::lockfree::queue<LogMessage *> q{1000};
    std::atomic<bool> stop;
    std::atomic<bool> finished;
};

// Logger main class
// Make sure you place an instance of this logger inside the heap! No STACK use!
XLogger::XLogger(const std::string &id)
  : id(id), min_loglvl(0), strip_msg(true), to_strip({"\n", "\t", " ", "\r"}), time_format("%d.%m - %T") {

    // early in initalization-phase - exit hard!
    if (log_map.find(id) != log_map.end())
        _wrongIDerror(id);

    backends.clear();
    log_map[id] = this;
    set_loglvl_desc(0, "?");

    io = new LogIO();
    CreateXLoggerThread(this);
}

void XLogger::_wrongIDerror(const std::string &fail_id) {
    std::cerr << "[XLogger] Somewho asks for a logger called: " << fail_id << std::endl;
    std::cerr << "[XLogger] NO SUCH LOGGER FOUND - GO FIX YOUR CODE!" << std::endl;
    std::cerr << "[XLogger] HINT: most likely you did not init XLogger, try XLogger::quick_init()" << std::endl;
    std::cerr << "[XLogger] critical! exiting!" << std::endl;
    exit(1);
}

void XLogger::stop() {
    io->stop.exchange(true);
}

// cleaning up / deleting backends
XLogger::~XLogger() {
    stop();
    while(!io->finished.load()) {
        sleep_for(10ms);
    }
    consume();
    for (BaseLoggerBackend *back : backends) {
        back->cleanup();
        delete back;
    }
    log_map.erase(id);
    delete io;
}

std::unique_ptr<XLogger> XLogger::quick_init(
        const std::string &newlogid, const std::string &console_format, const int &minloglvl, bool strip) {
    auto xlog = std::make_unique<XLogger>(newlogid);
    ConsoleBackend *xlog_console = new ConsoleBackend(newlogid);
    xlog->add_backend(xlog_console, console_format);

    xlog->set_loglvl_desc(10, "ERR");
    xlog->set_loglvl_desc(7, "W");
    xlog->set_loglvl_desc(5, "i");
    xlog->set_loglvl_desc(3, "DEBUG");
    xlog->set_loglvl_desc(1, "PROFILE");

    xlog->set_min_loglvl(minloglvl);
    xlog->set_msg_stripping(strip);

    return xlog;
}

void XLogger::add_filename_filter(const std::string &fn) {
    fn_filter.insert(fn);
}

void XLogger::remove_filename_filter(const std::string &fn) {
    fn_filter.erase(fn_filter.find(fn));
}

void XLogger::set_msg_stripping(const bool &strip_msg) {
    this->strip_msg = strip_msg;
}

void XLogger::set_msg_stripping(const bool &strip_msg, const std::vector<std::string> &trims) {
    set_msg_stripping(strip_msg);
    to_strip = trims;
}

// adding a backend
void XLogger::add_backend(BaseLoggerBackend *back, const std::string &tmpl, bool verbose) {
    if (tmpl == "")
        back2tmpl[back] = "[%%FANCYLVL%%] %%TIME%% (%%FILE%%:%%LINE%% call: %%FUNC%%) || %%MSG%%\n";
    else
        back2tmpl[back] = tmpl;

    backends.push_back(back);
    back->init();
    if (verbose)
        back->write("[XLogger] Initializing backend (" + back->name + ") for logger: " + id + "\n");
}

// static global getter for the registered XLoggers
XLogger *XLogger::get(const std::string &id) {
    if (log_map.find(id) != log_map.end())
        return log_map[id];

    _wrongIDerror(id);
    return NULL;
}

// static to check, if loglevel is high enough to log the msg
bool XLogger::check_loglevel(const int &lvl, const std::string &id) {
    return (XLogger::get(id)->min_loglvl <= lvl);
}

// accept %%MSG%% , %%TIME%%, %%FANCYLVL%%, %%LOGLVL%%, %%FILE%% , %%FUNC%% ,
// %%LINE%%
void XLogger::set_logging_template(BaseLoggerBackend *back, const std::string &tmpl) {
    back2tmpl[back] = tmpl;
}

// %%TIME%% is formated according to set_time_format() and strftime()
void XLogger::set_time_format(const std::string &format) {
    time_format = format;
}

// set description to be used for loglvl
void XLogger::set_loglvl_desc(int loglvl, const std::string &desc) {
    lvl2desc[loglvl] = desc;
}

// get loglvl description, decrease lvl gradually to 0, if not found.
std::string XLogger::get_fancy_level(int lvl) {
    while (lvl2desc.find(lvl) == lvl2desc.end())
        lvl--;
    return lvl2desc[lvl];
}

void XLogger::set_min_loglvl(int loglvl) {
    XLogger::min_loglvl = loglvl;
}

int XLogger::get_min_loglvl() const {
    return XLogger::min_loglvl;
}

// render the message
std::string XLogger::render_msg(BaseLoggerBackend *back, LogMessage *lm) {
    std::string out(back2tmpl[back]);
    std::string tmp_time;
    struct tm timeinfo;

    size_t pos;
    // actual msg
    while ((pos = out.find("%%MSG%%")) != std::string::npos)
        out.replace(pos, 7, lm->msg.str());

    // date time string
    while ((pos = out.find("%%TIME%%")) != std::string::npos) {
        if (tmp_time.empty()) {
            rawtime = time(NULL);
            memset(&buf, 0, DATE_TIME_BUF_LEN);
            localtime_r(&rawtime, &timeinfo);
            ::strftime(buf, DATE_TIME_BUF_LEN - 1, time_format.c_str(), &timeinfo);
            tmp_time = std::string(buf);
        }
        out.replace(pos, 8, tmp_time);
    }

    // time in millisecs
    while ((pos = out.find("%%TIMEMS%%")) != std::string::npos)
        out.replace(pos, 10, str(getTimestampMs()));

    // log level
    while ((pos = out.find("%%LOGLVL%%")) != std::string::npos)
        out.replace(pos, 10, str(lm->loglevel));

    // fancy log level
    while ((pos = out.find("%%FANCYLVL%%")) != std::string::npos)
        out.replace(pos, 12, get_fancy_level(lm->loglevel));

    // filename where the logger was called
    while ((pos = out.find("%%FILE%%")) != std::string::npos)
        out.replace(pos, 8, lm->fn);

    // line where the logger was called
    while ((pos = out.find("%%LINE%%")) != std::string::npos)
        out.replace(pos, 8, str(lm->line));

    // function from which context the logger was called
    while ((pos = out.find("%%FUNC%%")) != std::string::npos)
        out.replace(pos, 8, lm->func);

    return out;
}

// log message (including meta data)
void XLogger::log_msg(LogMessage *lm) {
    // if not fn_filter.empty() show only msg from given files
    if (!fn_filter.empty() && fn_filter.find(lm->fn) == fn_filter.end())
        return;

    // if loglvl below min_loglvl, discard!
    if (lm->loglevel < min_loglvl)
        return;

    io->q.push(lm);
}

void XLogger::io_worker(ThreadContext* context) {
    io->stop = false;
    context->notifyReady();

    while (!io->stop.load()) {
        if (consume() < 1) {
            sleep_for(10ms);
        }
    }
    io->finished.exchange(true);
}

void XLogger::flush() {
    // dummy to ensure all message are actually written to disk, when queue is empty
    io->q.push(nullptr);

    // sleep until queue is empty
    while (!io->q.empty())
        sleep_for(10ms);
}

int XLogger::consume() {
    return io->q.consume_all([&](LogMessage *lm) {
        // flush() may push nullptr, to ensure all messages are written to disk
        if (UNLIKELY(!lm))
            return;

        // render and write to all backends
        for (BaseLoggerBackend *back : backends)
            back->write(render_msg(back, lm));
        delete lm;
    });
}
