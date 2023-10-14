#pragma once

#include <chrono>
#include <algorithm>

enum class BtnClk { NONE, SINGLE, DOUBLE, TRIPPLE, LONG, CAM_RESET = 255 };

// detect multi-click on chest button
BtnClk checkBtnEvent(const float &state) {
    using namespace std::chrono;
    using namespace std::chrono_literals;
    using clock = std::chrono::steady_clock;

    bool btn = (state > 0);

    // time between first & last click to be considered tripple-click (1.5s)
    static constexpr auto multiClickTimeout{500ms};
    // detect long clicks (>3s, shutdown)
    static constexpr auto holdTimeout{3s};

    static bool prev_btn{false};
    static int btnCount{0};
    static time_point<clock> btnStart;

    auto ret{BtnClk::NONE};
    auto timeNow{clock::now()};

    if (btn) {
        if (!prev_btn) {
            ++btnCount;
            btnStart = timeNow;
        } else {
            if ((timeNow - btnStart) > holdTimeout) {
                btnCount = 0;
                btnStart = timeNow;
                ret = BtnClk::LONG;
            }
        }
    } else if ((timeNow - btnStart) > multiClickTimeout) {
        switch (std::min(btnCount, 6)) {
            case 1:
                ret = BtnClk::SINGLE;
                break;
            case 2:
                ret = BtnClk::DOUBLE;
                break;
            case 3:
                ret = BtnClk::TRIPPLE;
                break;
            case 6:
                ret = BtnClk::CAM_RESET;
                break;
        }

        btnCount = 0;
    }

    prev_btn = btn;

    return ret;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
