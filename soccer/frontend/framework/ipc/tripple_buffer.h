#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

#include <boost/thread/thread.hpp>


namespace ipc {

template<typename T>
class TrippleBuffer {
    using ipc_lock = boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>;

public:
    TrippleBuffer() = default;

    // returns reference to data produced by other process
    const T &producedData() const {
        return buffers[frontIdx];
    }

    // produce data from other process
    bool tryProduce() {
        ipc_lock lock(middleMutex);
        return switchNewMiddleToFront(newData);
    }

    // produce data from other process (with timeout)
    bool timedProduce(int timeoutMs) {
        ipc_lock lock(middleMutex);
        boost::posix_time::ptime timeout = boost::get_system_time() 
            + boost::posix_time::milliseconds(timeoutMs);

        bool newData = 
            (this->newData) ? true 
                            : middleOld.timed_wait(lock, timeout,
                                [this]() { return this->newData; });

        return switchNewMiddleToFront(newData);
    }

    void waitForNewData() {
        ipc_lock lock(middleMutex);

        middleOld.wait(lock, [this]() { return this->newData; }); 

        switchFrontAndMiddle();
    }


    // returns reference to data that will be consumed from current process
    T &consumedData() {
        return buffers[backIdx];
    }

    // consume data from current process
    void consume() {
        unsigned char oldBack = backIdx;
        {
            ipc_lock lock(middleMutex);
            backIdx = middleIdx;
            middleIdx = oldBack;
            newData = true;
        }
        middleOld.notify_one();
    }

private:
    T buffers[3];

    unsigned char backIdx   = 0; //< Index of current back buffer
    unsigned char middleIdx = 1; //< Index of current middle buffer
    unsigned char frontIdx  = 2; //< Index of current front buffer

    bool newData = false; //< if data in middle buffer is newer than front buffer

    boost::interprocess::interprocess_mutex middleMutex; //< mutex for access to the middle buffer
    boost::interprocess::interprocess_condition middleOld; 

    void switchFrontAndMiddle() {
        unsigned char oldFront = frontIdx;
        frontIdx = middleIdx;
        middleIdx = oldFront;
        newData = false;
    }

    bool switchNewMiddleToFront(bool newData) {
        if (newData) {
            switchFrontAndMiddle();
        }
        return newData;
    }

};
    

}; // namespace ipc

// vim: set ts=4 sw=4 sts=4 expandtab:
