#include "clock.h"

#include <chrono>

#include "../logger/logger.h"
#include "../util/assert.h"

/**
 * virtual base class for clock implementations
 */
class Clock {
public:
    Clock() = default;
    virtual ~Clock() = default;
    virtual TimestampMs getTimeMs() = 0;
    virtual microTime getTimeUs() = 0;
    virtual void setTime(const TimestampMs &t) = 0;
};

/**
 * "real" clock, reports time since framework start
 */
class ClockRealtime : public Clock {
public:
    using clock_t = std::chrono::steady_clock;

    ClockRealtime() : start(clock_t::now()) {}

    TimestampMs getTimeMs() override {
        std::chrono::duration<float, std::milli> t = clock_t::now() - start;
        return t.count();
    }

    microTime getTimeUs() override {
        std::chrono::duration<float, std::micro> t = clock_t::now() - start;
        return t.count();
    }

    void setTime(const TimestampMs &t) override {
       jsassert(false) << __PRETTY_FUNCTION__ << " not implemeted";
    };

private:
    std::chrono::time_point<clock_t> start;
};

/**
 * "simulator" clock, always returns value set by setGlobalTimeFromSimulation()
 */
class ClockSimulation : public Clock {
public:
    TimestampMs getTimeMs() override { return time; }

    microTime getTimeUs() override { return time * 1000; }

    void setTime(const TimestampMs &t) override { time = t; };

private:
    TimestampMs time{0};
};

/**
 * global pointer used to switch between clocks
 * standalone behavior always uses simulator clock
 */
#ifdef STANDALONE_BEHAVIOR
static Clock *clk{new ClockSimulation};
#else
static Clock *clk{new ClockRealtime};
#endif

microTime getMicroTime() {
    return clk->getTimeUs();
}

TimestampMs getTimestampMs() {
    return clk->getTimeMs();
}

void setGlobalTimeFromSimulation(TimestampMs t) {
    clk->setTime(t);
}

void useSimulatorClock() {
    delete clk;
    clk = new ClockSimulation;
    LOG_INFO << "Using simulator clock.";
}
