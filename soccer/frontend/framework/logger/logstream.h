#pragma once

#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <string_view>
#include <unordered_map>

class XLogger;

class LogMessage {
public:
    int loglevel;
    int line;
    std::string_view fn;
    std::string_view func;
    std::stringstream msg;

    LogMessage(const int &loglevel, const int &line, const std::string_view &fn, const std::string_view &func)
      : loglevel(loglevel), line(line), fn(fn), func(func) {}
};

// LogStream object which performs the piping magic,
// instanciated on every LOG(lvl, LOGID) macro call,
// renders and delivers the the log message during object desctruction
// TODO: alot of performance optimization possible, e.g.:
// - static stringstream (violates thread-safety)
class LogStream {
private:
    XLogger *obj;
    LogMessage *lm;

public:
    typedef std::basic_ostream<char, std::char_traits<char>> tCout;
    typedef tCout &(*tEndl)(tCout &);

    LogStream(XLogger *logobj, const int &loglvl, const int &line, const std::string_view &fn, const std::string_view &func);
    ~LogStream();

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    // handle piping-in 'std::endl'
    LogStream &operator<<(tEndl fnc);

    // default pipe-to handler
    template<class T>
    LogStream &operator<<(const T &obj) {
        lm->msg << obj;
        return *this;
    }
};

class FakeLogStream {
public:
    template<class T>
    FakeLogStream &operator<<(const T &) {
        return *this;
    }
};

/** various dumping-to-logger helpers to allow easy piping to LogStream() **/
template<class T>
inline LogStream &operator<<(LogStream &out, const std::set<T> &v) {
    for (const auto &i : v)
        out << i << "  ";
    return out;
}

template<class T>
inline LogStream &operator<<(LogStream &out, const std::vector<T> &v) {
    for (const auto &i : v)
        out << i << "  ";
    return out;
}

template<class K, class V>
inline LogStream &operator<<(LogStream &out, const std::map<K, V> &m) {
    for (const auto &i : m)
        out << i.first << ": " << i.second << " ";
    return out;
}

template<class K, class V>
inline LogStream &operator<<(LogStream &out, const std::unordered_map<K, V> &m) {
    for (const auto &i : m)
        out << i.first << ": " << i.second << " ";
    return out;
}

template<class K, class V>
inline LogStream &operator<<(LogStream &out, const std::pair<K, V> &p) {
    out << "key: " << p.first << " val: " << p.second;
    return out;
}

