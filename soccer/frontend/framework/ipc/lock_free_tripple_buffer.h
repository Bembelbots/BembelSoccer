#pragma once

#include <atomic>
#include <thread>


namespace ipc {
    
template<typename T>
class LockFreeTrippleBuffer {

    inline const T &front() {
        return buffers[frontIdx]; 
    }

    inline bool switchFrontAndMiddle() {
        byte newFront = middle.exchange(frontIdx); 
        frontIdx = getIdx(newFront);
        return hasNewData(newFront);
    }

    bool waitForNewData(int maxWaitTimeMs, int intervalMs) {
        const auto interval = std::chrono::milliseconds(intervalMs);
        bool newData = switchFrontAndMiddle();
        while (maxWaitTimeMs > 0 && !newData) {
            std::this_thread::sleep_for(interval);
            newData = switchFrontAndMiddle();
            maxWaitTimeMs -= intervalMs;
        }
        return newData;
    }

    inline T &back() {
        return buffers[backIdx];
    }

    inline void switchBackAndMiddle() {
        byte prev = middle.exchange(backMask());
        backIdx = getIdx(prev);
    }

private:
    using Mask = unsigned char;
    using byte = unsigned char;
    using MiddleHandle = std::atomic<byte>;

    static constexpr byte IDX_MASK = 0b11;
    static constexpr byte NEW_DATA_MASK = 0b100;

    T buffers[3];

    byte backIdx = 0; //< Index of current back buffer
    MiddleHandle middle = 1;
    byte frontIdx = 2; //< Index of current front buffer

    static inline Mask backMask() {
        return backIdx | NEW_DATA_MASK; 
    }

    static inline byte idx(byte middle) {
        return middle & IDX_MASK;
    }

    static inline bool hasNewData(byte middle) {
        return middle & NEW_DATA_MASK;
    }

};

}; // namespace ipc
