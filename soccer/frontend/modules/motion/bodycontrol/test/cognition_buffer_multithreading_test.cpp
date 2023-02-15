#include <bodycontrol/bodyblackboard.h>
#include <bodycontrol/internals/cognition_buffer.h>
#include <bodycontrol/environment/logger.h>
#include <thread>
#include <unistd.h>
#include <mutex>

#define TEST_ITERATIONS 1E3

mutex cognition_buffer_test_mutex;

void read_thread(CognitionBuffer * cb){
    int last_ts = -1;
    int counter = 0;
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);
    for (int i = 0; i < TEST_ITERATIONS/3; i++){
        auto foo = cb->readLast();
        if (foo.timestamp_ms == last_ts)
            counter++;
        last_ts = foo.timestamp_ms;
        usleep(30);
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);
    double time_s = end_time.tv_sec - start_time.tv_sec + (end_time.tv_nsec-start_time.tv_nsec)*1E-9;
    
    cognition_buffer_test_mutex.lock();
    LOG_INFO << "bad reads: " << counter << "/" << TEST_ITERATIONS << EL;
    LOG_INFO << "Rtime: " << time_s*1E6/TEST_ITERATIONS*3 << "us" << EL;
    cognition_buffer_test_mutex.unlock();
    
}

void write_thread(CognitionBuffer * cb){
    BodyBlackboard bb;
    bb.timestamp_ms = 0;
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);
    for (int i = 1; i <= TEST_ITERATIONS; i++){
        bb.timestamp_ms = i;
        cb->write(bb);
        usleep(10);
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time);
    double time_s = end_time.tv_sec - start_time.tv_sec + (end_time.tv_nsec-start_time.tv_nsec)*1E-9;
    cognition_buffer_test_mutex.lock();
    LOG_INFO << "Wtime: " << time_s*1E6/TEST_ITERATIONS << "us" << EL;
    cognition_buffer_test_mutex.unlock();
}


void cognition_buffer_multithreading_test(){
    CognitionBuffer buffer;
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    thread read_t(read_thread, & buffer);
    thread write_t(write_thread, & buffer);
    read_t.join();
    write_t.join();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double time_s = end_time.tv_sec - start_time.tv_sec + (end_time.tv_nsec-start_time.tv_nsec)*1E-9;
    //time_s -= (TEST_ITERATIONS * 1e-6);
    double avg = time_s / double(TEST_ITERATIONS) *1E9;
    LOG_INFO << "CognitionBuffer total wall time: " << time_s*1000 << "ms" EL;
    LOG_INFO << "CognitionBuffer average wall time: " << avg/1000 << "us" EL;
}
