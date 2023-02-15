#pragma once
#include <thread>
#include <chrono>

class FrameScope {
public:
    FrameScope(std::chrono::milliseconds frequency)
        :   start(std::chrono::high_resolution_clock::now()) {
        using namespace std::chrono_literals;
        frametime = 1000ms / frequency.count();
    }

    ~FrameScope() {
        using namespace std::chrono;
        using namespace std::chrono_literals;
        auto end = high_resolution_clock::now();
        auto timetaken = duration_cast<milliseconds>(end - start);
        auto remainder = frametime - timetaken;
        std::this_thread::sleep_for(std::max(0ms, remainder)); 
    }

private:
    std::chrono::system_clock::time_point start;
    std::chrono::milliseconds frametime;
};
