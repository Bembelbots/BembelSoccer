#pragma once

#include <chrono>
#include <vector>
#include <map>
#include <stack>
#include <string>
#include <cassert>


class Timer {

    typedef std::chrono::steady_clock::time_point tpoint;
    typedef std::chrono::steady_clock::duration duration;

public:

    static inline void start(const std::string &name) {
        Timer &timer = getTimer(name);
        timer.start();
        activeTimers.push(&timer);
    }

    static inline void stop() {
        Timer *timer = activeTimers.top();
        timer->end();
        activeTimers.pop();
    }

    /**
     * Returns the timer with the given name.
     * If no timer with this name exists a new one will be created.
     */
    static inline Timer &getTimer(const std::string &name) {
        if (timers.find(name) == timers.end()) {
            timers.insert(std::pair<const std::string, Timer>(name, Timer()));
        }
        return timers.at(name);
    }

    /**
     * Returns the timer with the given name.
     * This function wont create the timer if it doesnt exist already.
     */

    static inline Timer &getExistingTimer(const std::string &name) {
        auto lookup = timers.find(name);
        assert(lookup != timers.end());
        return lookup->second;
    }

    static void saveAsCsv(const std::string &fname);

    static std::string str(const std::string &seperator);


    inline void start() {
        startTime = std::chrono::steady_clock::now();
    }

    inline void end() {
        endTime = std::chrono::steady_clock::now();
        unsigned int m = std::chrono::duration_cast<std::chrono::milliseconds>
                         (endTime - startTime).count();
        meassurements.push_back(m);
    }

    inline std::vector<unsigned int> getMeassurements() const {
        return meassurements;
    }

    inline unsigned int lastMeassurement() const {
        assert(!meassurements.empty());
        return meassurements.back();
    }


private:

    static std::map<const std::string, Timer> timers;
    static std::stack<Timer *> activeTimers;

    tpoint startTime;
    tpoint endTime;

    std::vector<unsigned int> meassurements;

};

// vim: set ts=4 sw=4 sts=4 expandtab:
