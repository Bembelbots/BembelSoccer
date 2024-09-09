#pragma once
#include <boost/program_options.hpp>
#include <memory>

class XLogger;
class ConsoleBackend;
class System;

struct Engine {
    virtual int run(bool docker, System *system);
    virtual ~Engine(){}
};

class System
{
public:
    bool docker{false};
    
    bool init(int argc, char *argv[]);
    int run();
    void stop();
    
    template<typename T, typename ...Args>
    void set_engine(Args&&... args)
    {
        engine = std::make_unique<T>(std::forward<Args>(args)...);
    }

    static bool is_running;
private:
    std::unique_ptr<Engine> engine;
    std::unique_ptr<XLogger> xlog;
    ConsoleBackend* xlog_console;
    boost::program_options::variables_map cli;

    void init_logger();

    bool init_cli(int argc, char *argv[]);
};
