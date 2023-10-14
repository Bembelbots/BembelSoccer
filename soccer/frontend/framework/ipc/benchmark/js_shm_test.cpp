#include <shared/backend/bbsharedmemory.h>
#include <shared/ipc/time.h>
#include <shared/utils/csv_file.h>

#include <cstdlib>
#include <iostream>
#include <thread>

static constexpr int frameCount = 1000;
static constexpr int frameLenMs = 10;

static const std::string shmName = "/channelTestShm";
static const std::string resultFile = "js_shm_test.csv";

struct IntMsg {
    int x;
    sysMicroTime timestamp;
    int tick = 0;
};

static void producer() {
    // Wait for creation of shm
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    JsSharedMemoryWriter<IntMsg> ch(shmName, sizeof(IntMsg), false);

    for (int i = 0; i < frameCount; i++) {
        IntMsg msg;        
        msg.x = i;
        msg.timestamp = getSystemTimestamp();
        msg.tick = i;
        ch.write(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(frameLenMs));
    }
}

static void consumer() {
    JsSharedMemoryReader<IntMsg> ch(shmName, sizeof(IntMsg), true); 

    CsvFile<sysMicroTime, 4> result(resultFile, 
            {"myVal", "recVal", "sendTime", "recvTime"}, frameCount);

    for (int i = 0; i < frameCount; i++) {
        IntMsg msg;
        while (!ch.read(&msg)) {}

        sysMicroTime recvTime = getSystemTimestamp();
        result.addRow({i, msg.x, msg.timestamp, recvTime});
    }
}


int main() {
     
    int pid = fork();
    if (pid < 0) {
        std::cout << "fork error" << std::endl;
    } else if (pid == 0) {
        producer();
    } else {
        consumer();
    }

    return 0;
}
