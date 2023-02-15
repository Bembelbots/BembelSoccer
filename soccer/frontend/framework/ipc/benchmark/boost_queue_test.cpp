#include <shared/ipc/time.h>
#include <shared/utils/csv_file.h>

#include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <thread>



using namespace boost::interprocess;

static constexpr int frameCount = 1000;
static constexpr int frameLenMs = 10;

static const std::string shmName = "/channelTestShm";
static const std::string resultFile = "message_queue.csv";


struct IntMsg {
    int x;
    sysMicroTime timestamp;
};

static void producer() {
    message_queue::remove(shmName.c_str());
    message_queue mq(create_only, shmName.c_str(), 1, sizeof(IntMsg));
    for (int i = 0; i < frameCount; i++) {
        IntMsg msg;
        msg.x = i;
        msg.timestamp = getSystemTimestamp();
        mq.send(&msg, sizeof(IntMsg), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(frameLenMs));
    }

}

static void consumer() {

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    message_queue mq(open_only, shmName.c_str()); 
    CsvFile<sysMicroTime, 4> result(resultFile, 
            {"myVal", "recVal", "sendTime", "recvTime"}, frameCount);
    for (int i = 0; i < frameCount; i++) {
        IntMsg msg;
        unsigned int recvSize;
        size_t priority;
        mq.receive(&msg, sizeof(IntMsg), recvSize, priority);
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
