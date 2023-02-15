#pragma once

#include <core/logger.h>
#include <core/logger/backends.h>

#include <sstream>


namespace stab {
    
class Log : public BaseLoggerBackend {

public:
    static Log &instance();

    Log();

    // Initialize logger. 
    // Run this function once at the start of the program before creating any
    // instances of Behavior.
    void initialize();

    void setActiveRobot(const std::string &robot) { this->robot = robot; }

    void write(const std::string &) override;

    std::string flush();

private:
    std::string robot;

    std::stringstream ss;

};

} // namespace stab

// vim: set ts=4 sw=4 sts=4 expandtab:
