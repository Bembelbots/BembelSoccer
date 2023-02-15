#pragma once
#include <sstream>
using std::stringstream;
using std::string;


class LogCollector;
LogCollector * motion_logger_cout_singelton = 0;
LogCollector * motion_logger_network_singleton = 0;

#define M_LOG MotionLogger(motion_logger_cout_singelton)
#define M_NET MotionLogger(motion_logger_network_singleton)

class LogCollector {
    public:
    LogCollector(LogCollector *&singleton_){singleton_ = this;}
    ~LogCollector(){singleton = 0;}


    void collect(string msg){
        stream << msg << "\n";
    }

    string getContent(){
        string msg = stream.str();
        stream.str(string());
        stream.clear();
        return msg;
    }

    private:
    stringstream stream;
    LogCollector * singleton;

};


class MotionLogger: public stringstream {
    public:
    MotionLogger(LogCollector * collector):collector(collector){}
    ~MotionLogger(){ if (collector) collector->collect(stringstream::str()); }
    private:
    LogCollector * collector;
};
