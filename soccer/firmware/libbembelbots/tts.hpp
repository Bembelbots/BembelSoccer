#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <libspeechd.h>


/*
 *  text-to-speech output using speech-dispatcher
 */
class TTSSpeechd {
public:
    explicit TTSSpeechd(std::string_view client_name, std::string_view connection_name="main") {
        conn = spd_open(client_name.data(), connection_name.data(), nullptr, SPD_MODE_SINGLE);
        if (conn)
            spd_set_language(conn, "en");
        else
            std::cerr << __PRETTY_FUNCTION__ << " - failed to connect to speech dispatcher." << std::endl;
    }

    ~TTSSpeechd() {
        std::scoped_lock l(mtx);
        if (conn)
            spd_close(conn);
    }

    bool say(const std::string &txt, const SPDPriority &prio=SPD_TEXT) {
        if (conn) {
            std::scoped_lock l(mtx);
            return spd_say(conn, prio, txt.c_str()) > 0;
        }
        return true;
    }

private:
    SPDConnection* conn;
    std::mutex mtx;
};


// vim: set ts=4 sw=4 sts=4 expandtab:
