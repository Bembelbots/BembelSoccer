#include <shared/ipc/tripple_buffer.h>
#include <shared/ipc/shared_memory.h>
#include <shared/ipc/time.h>

#include <shared/utils/csv_file.h>

#include <cassert>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <random>
#include <thread>


static constexpr int frameCount = 1000;
static constexpr int frameLenMs = 10;

static const std::string shmName = "/trippleBufferTestShm";
static const std::string stopName = "/testStopShm";
static const std::string resultFile = "tripple_buffer_test.csv";


static constexpr size_t L2CacheSize = 512 << 10; // 512kb l2 cache
static constexpr size_t L1CacheSize = 16 << 10;  // 16kb l1 cache
static constexpr size_t cacheLine = 64;

struct Message {
    int tick;
    sysMicroTime timestamp;
    unsigned char blob[L1CacheSize];
};


static void troublemaker() {

    std::array<uint64_t, L2CacheSize> from;
    /* std::array<uint64_t, cacheSize> to; */

    std::random_device dev;
    std::mt19937_64 rng(dev());
    std::uniform_int_distribution<std::mt19937_64::result_type> dist(0, L2CacheSize-1);

    // fill buffer with random values
    volatile uint64_t *data = from.data(); // prevent compiler from optimizing;
    for (;;) {
        for (size_t pos = 0; pos < from.size(); pos += cacheLine / sizeof(uint64_t)) {
            data[pos] = dist(rng); 
        }
    }

}

static void producer() {
    ipc::SharedMemory<ipc::TrippleBuffer<Message>> ch(shmName, true);
    ipc::SharedMemory<ipc::TrippleBuffer<bool>> stop(stopName, true);

    
    for (int tick = 0;; tick++) {
        ch->back() = {tick, getSystemTimestamp(), {}};
        ch->switchBackAndMiddle();

        stop->trySwitchFrontAndMiddle();
        if (stop->front()) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(frameLenMs));
    }

}

static void consumer() {

    // Wait for creation of shm
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ipc::SharedMemory<ipc::TrippleBuffer<Message>> ch(shmName, false);
    ipc::SharedMemory<ipc::TrippleBuffer<bool>> stop(stopName, false);

    CsvFile<sysMicroTime, 4> result(resultFile, 
            {"myVal", "recVal", "sendTime", "recvTime"}, frameCount);

    for (int i = 0; i < frameCount; i++) {
        bool longFetch = false;
        while (true) {
            bool ok = ch->timedSwitchFrontAndMiddle(12);
            if (ok) {
                break;
            } else {
                longFetch = true;
                std::cout << "Fetching data took too long" << std::endl;
            }
        }

        Message msg = ch->front();
        sysMicroTime recvTime = getSystemTimestamp();
        if (longFetch) {
            std::cout << "time passed: " <<  recvTime - msg.timestamp << std::endl;
        }
        result.addRow({i, msg.tick, msg.timestamp, recvTime});

        if (msg.tick == frameCount-1) {
            stop->back() = true;
            stop->switchBackAndMiddle();
            return;
        }
    }
}

int main() {
     
    int troublemakerId = 0;
    for (int i = 0; i < 2; i++) {

        int pid = fork();

        if (pid < 0) {
            std::cout << "fork error" << std::endl;
            exit(1);
        } else if (pid == 0) {

            switch (i) {
            case 0: troublemaker(); break; 
            case 1: producer(); break;
            }

            exit(0);

        } else if (i == 0) {
            troublemakerId = pid;
        }
    }

    consumer();

    if (troublemakerId == 0) {
        std::cout << "Invalid process id";
    } else {
        kill(troublemakerId, SIGKILL);
    }

    return 0;
}
