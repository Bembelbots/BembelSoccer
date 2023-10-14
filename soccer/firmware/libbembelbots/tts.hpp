#include <future>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string_view>

/*
 * writes text to fifo /tmp/tts, which is read by mimic
 *
 * I/O happens asynchronously, unless say() is called again
 * before the previous write has finished (which should rarely
 * happen) or object is destroyed immediately
 */
class TTSFifo {
public:
    void say(const std::string txt) {
        static const std::filesystem::path fifo{"/tmp/tts"};

        f = std::async([=]() {
            if (!std::filesystem::is_fifo(fifo)) {
                std::cerr << __PRETTY_FUNCTION__ << " - TTS failed: " << fifo << " is not a named pipe" << std::endl;
                return;
            }

            try {
                // re-open file every time, since flush does not work
                std::ofstream ofs(fifo);
                ofs << txt << std::endl;
                ofs.close();
            } catch (std::exception &e) {
                // use cerr since lola backend has no logger
                std::cerr << __PRETTY_FUNCTION__ << " - TTS failed: " << e.what() << std::endl;
            }
        });
    }

private:
    std::future<void> f;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
