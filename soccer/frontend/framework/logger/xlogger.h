#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <memory>

#include "logstream.h"

class ThreadContext;
class BaseLoggerBackend;
class LogIO;

class XLogger;
extern void CreateXLoggerThread(XLogger *logger);

/** XLogger main class to access/configure/control the logger **/

class XLogger {
public:
    using tBackendList = std::vector<BaseLoggerBackend *>;
    using tBackendIter = tBackendList::iterator;
    using tLevelDescMap = std::unordered_map<int, std::string>;
    using tBackendTemplateMap = std::unordered_map<BaseLoggerBackend *, std::string>;
    using tXLoggerMap = std::unordered_map<std::string, XLogger *>;

    explicit XLogger(const std::string &id);
    ~XLogger();

    void stop();

    XLogger(const XLogger&) = delete;
    XLogger& operator=(const XLogger&) = delete;
    
    // static error handler (called on e.g., unknown LOGID)
    static void _wrongIDerror(const std::string &fail_id);

    // keeps all available loggers 'name' -> 'XLogger*'
    static tXLoggerMap log_map;

    // get a specific logger instance by 'id'
    static XLogger *get(const std::string &id);

    // check, if 'lvl' is an accepted one (runtime AND compiletime)
    static bool check_loglevel(const int &lvl, const std::string &id);

    // quick init logger (e.g., for testing)
    [[nodiscard]] static std::unique_ptr<XLogger> quick_init(const std::string &newlogid, const std::string &console_format = "[%%FANCYLVL%%] %%MSG%%\n",
            const int &minloglvl = 3, bool strip = false);

    // add backend to output log-messages to
    void add_backend(BaseLoggerBackend *back, const std::string &tmpl = "", bool verbose = true);

    // set logging template by backend
    void set_logging_template(BaseLoggerBackend *back, const std::string &tmpl);

    // strftime time format configuration
    void set_time_format(const std::string &format);

    // bind post-called functions/methods to specific log-lvls
    void set_loglvl_desc(int loglvl, const std::string &desc);

    // strip each message according to "trims" or simply whitespaces, newlines
    void set_msg_stripping(const bool &strip_msg);
    void set_msg_stripping(const bool &strip_msg, const std::vector<std::string> &trims);

    // filter log-messages by originating file
    void add_filename_filter(const std::string &fn);
    void remove_filename_filter(const std::string &fn);

    // runtime log-lvl set/get
    void set_min_loglvl(int loglvl);
    int get_min_loglvl() const;

    // actual log-msg handling methods
    void log_msg(LogMessage *lm);

    // block until all messages are written (should only be in special cases)
    void flush();

private:
    friend void CreateXLoggerThread(XLogger *);
    static constexpr int DATE_TIME_BUF_LEN{64};

    tBackendList backends;
    tLevelDescMap lvl2desc;
    tBackendTemplateMap back2tmpl;

    std::string id;

    int min_loglvl;

    bool strip_msg;
    std::vector<std::string> to_strip;
    std::set<std::string_view> fn_filter;

    std::string time_format;
    time_t rawtime;
    char buf[DATE_TIME_BUF_LEN];

    LogIO *io;

    // internal final log-message renderer
    // TODO: alot performance potential
    // -> preprocess template to avoid find/replace calls on each log-msg
    std::string render_msg(BaseLoggerBackend *back, LogMessage *lm);
    std::string get_fancy_level(int lvl);

    void io_worker(ThreadContext*);
    int consume();
};
