#include <bodycontrol/internals/submodule.h>
#include <unistd.h>

class Timestamp : public SubModule {
public:
    Timestamp(){
        clock_gettime(CLOCK_MONOTONIC, &start_time);
    }

public:
    SubModuleReturnValue step(BodyBlackboard * bb) override {
        clock_gettime(CLOCK_MONOTONIC, &now);
        int timestamp_ms = ((now.tv_nsec-start_time.tv_nsec)/1E6)
            +((now.tv_sec-start_time.tv_sec)*1E3);
        bb->timestamp_ms = timestamp_ms;
        return RUNNING;
    }

private:
    struct timespec now, start_time;
};




