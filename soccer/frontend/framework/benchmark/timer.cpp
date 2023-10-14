#include "timer.h"

#include <sstream>
#include <fstream>

using namespace std;


map<const string, Timer> Timer::timers = map<const string, Timer>();

stack<Timer *> Timer::activeTimers = stack<Timer *>();

void Timer::saveAsCsv(const string &fname) {
    ofstream file(fname);
    file << str(",");
    file.close();
}

string Timer::str(const string &seperator=" ") {
    stringstream ss;

    for (const auto &namedTimer: timers) {
        const string &name = namedTimer.first;
        const Timer &timer = namedTimer.second;

        ss << name << seperator;

        for (const unsigned int m : timer.getMeassurements()) {
            ss << m << seperator;
        }

        ss.seekp(-1, ios_base::end); // Remove trailing seperator

        ss << "\n";
    }

    return ss.str();
}

// vim: set ts=4 sw=4 sts=4 expandtab:

