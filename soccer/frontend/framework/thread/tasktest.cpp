#include "task.h"
#include "taskrunner.h"
#include <thread>
#include <unistd.h>
#include <csignal>
#include <cassert>
#include <chrono>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

using namespace spdlog;
using namespace std::chrono;
using std::this_thread::sleep_for;
using hrc = std::chrono::high_resolution_clock;

/* classes {{{1 */

/*
class Foo {
    TaskOutput<Image, States> output;
    ThreadPool pool;
};

class Bar {
    TaskInput<Image, States, States::SECOND> input;
};

class Baz {
    TaskInput<Image, States, States::THIRD> input;
};
*/

enum States {
    FIRST = 0,
    SECOND,
    THIRD,
    //FOURTH,
    NO_OF_STATES
};

struct Image {
    int id = -1;
    int width = 640;
    int height = 480;
    uint8_t *data = nullptr;
};

template<typename OStream>
OStream& operator<< (OStream& out, const Image& image) {
    out << "image (" << image.id << ")";
    return out;
}

/* global variables {{{1 */

static constexpr int images_per_second = 30;
static constexpr int num_cameras = 2;
static uint32_t num_workers = std::thread::hardware_concurrency() / num_cameras;

static constexpr States complete_state = States::NO_OF_STATES;
static constexpr milliseconds images_freq = milliseconds(1000) / images_per_second;

std::vector<std::thread> threads;
ThreadPool pool;
TaskChannel<Task<Image, States>> channel;
bool running = true;

/* functions {{{1 */

void signalHandler( int signum ) {
    std::cout << std::endl;
    warn("Interrupt signal ({}) received.", signum);

    running = false;
    
    channel.close(); 
    pool.stop();
    debug("channel and pool stopped");

    for(auto &thread : threads) {
        thread.join();
    }

    exit(signum);  
}

void coordinator(States complete_state) {
    debug("coordinator started");
    static int id = 0;

    while(running) {
        auto tickstart = hrc::now();

        for(int i = 0; i < num_cameras; i++) {
            Image image;
            image.id = ++id;
            info("create task {} {}", id, image);
            channel.submit(id, complete_state, image);
        }

        for(int i = 0; i < num_cameras && running;) {
            for(auto task : channel.takeComplete()) {
                info("task complete {}", task->id);
                i++;
            }
        }

        if(!running) {
            break;
        }

        auto timetaken = hrc::now() - tickstart;
        auto expected = (tickstart + images_freq) - tickstart;

        if(timetaken > expected) {
            warn("dropped frames");
            continue;
        }

        auto rest = expected - timetaken;
        debug("sleep for: {}", rest.count());
        sleep_for(rest);
    }

    debug("coordinator stopped");
}

void taskworker(uint32_t threadid, States state) {
    debug("worker {} started", threadid);
    int listenid = channel.listen(state);

    while(running) {
        for(auto &ticket : channel.take(listenid)) {
            pool.add_task([ticket, threadid, state](){
                auto task = ticket->get_task();
                assert(task->current_state == state);
                if(threadid == num_workers) {
                    sleep_for(milliseconds(4));
                } else {
                    switch(state) {
                        case States::FIRST:
                            sleep_for(milliseconds(16));
                        break;
                        default:
                            sleep_for(milliseconds(1));
                        break;
                    }
                }
                debug("worker {} | task {} | {} | state {} | expected state {}",
                    threadid, task->id, task->data, task->current_state, state);
                //ticket->release();
            });
        }
    };

    debug("worker {} stopped", threadid);
}

int main() {
    spdlog::set_pattern("%^[%L]%$ %v");
    spdlog::set_level(level::debug);

    signal(SIGINT, signalHandler); 

    debug("starting {} workers", num_workers);

    for(uint32_t i = 0; i < num_workers; i++) {
        auto state = States(i % (size_t)complete_state);
        //auto state = States::FIRST;
        threads.emplace_back(taskworker, i + 1, state);
    }

    pool.start();

    threads.emplace_back(coordinator, complete_state);

    while(running) {
        sleep_for(milliseconds(100));
    }

    for (auto &thread : threads) {  
        std::cout <<thread.get_id() << std::endl;
    }
    debug("exiting main thread");

    return 0;
}
