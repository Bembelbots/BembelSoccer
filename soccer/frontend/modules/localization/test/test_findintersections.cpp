#include <core/util/mathtoolbox.h>
#include <core/util/tafel.h>
#include <worldmodel/pose/playingfield.h>
#include <worldmodel/pose/pose.h>

#include <vector>
#include <random>
#include <iostream>
#include <chrono>
#include <boost/math/constants/constants.hpp>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <boost/filesystem.hpp>

#include "../../motion/test/utilsForTesting.cpp"

static const std::vector<line_t> field_lines = {
    {{ 4.5f, -3.0f}, { 0.0f, -3.0f}}, //LINENAME_RIGHT
    {{ 0.0f, -3.0f}, {-4.5f, -3.0f}}, //LINENAME_RIGHT

    {{ 4.5f,  3.0f}, { 0.0f,  3.0f}}, //LINENAME_LEFT
    {{ 0.0f,  3.0f}, {-4.5f,  3.0f}}, //LINENAME_LEFT

    {{ 0.0f,  3.0f}, { 0.0f, -3.0f}}, //LINENAME_MIDDLE

    {{ 3.9f,  1.1f}, { 3.9f, -1.1f}}, //LINENAME_OPPONENT_PENALTY
    {{ 4.5f,  1.1f}, { 3.9f,  1.1f}}, //LINENAME_OPPONENT_PENALTY_LEFT
    {{ 4.5f, -1.1f}, { 3.9f, -1.1f}}, //LINENAME_OPPONENT_PENALTY_RIGHT

    {{-3.9f,  1.1f}, {-3.9f, -1.1f}}, //LINENAME_OUR_PENALTY
    {{-4.5f,  1.1f}, {-3.9f,  1.1f}}, //LINENAME_OUR_PENALTY_LEFT
    {{-4.5f, -1.1f}, {-3.9f, -1.1f}}, //LINENAME_OUR_PENALTY_RIGHT

    {{ 4.5f,  3.0f}, { 4.5f,  1.1f}}, //LINENAME_TOP
    {{ 4.5f,  1.1f}, { 4.5f, -1.1f}}, // "
    {{ 4.5f, -1.1f}, { 4.5f, -3.0f}}, //LINENAME_TOP

    {{-4.5f,  3.0f}, {-4.5f,  1.1f}}, //LINENAME_BOTTOM
    {{-4.5f,  1.1f}, {-4.5f, -1.1f}}, // "
    {{-4.5f, -1.1f}, {-4.5f, -3.0f}}, //LINENAME_BOTTOM

    {{ 2.7f,  0.05f}, { 2.7f, -0.01f}},  //LINENAME_OPPONENT_PENALTY_SHOOTMARK
    {{-2.7f,  0.05f}, {-2.7f, -0.05f}}   //LINENAME_OUR_PENALTY_SHOOTMARK
};

static constexpr float minVisibleLineLength = 0.6;
static constexpr float minInvisibleLineLength = 1.9;

static const std::vector<std::pair<Coord, MeasurementTypes>> intersections = {
    {{+4.5f, +3.0f}, L_CROSS_MEASUREMENT}, // Begin field corners
    {{+4.5f, -3.0f}, L_CROSS_MEASUREMENT},
    {{-4.5f, +3.0f}, L_CROSS_MEASUREMENT},
    {{-4.5f, -3.0f}, L_CROSS_MEASUREMENT},
    {{+0.0f, +3.0f}, T_CROSS_MEASUREMENT}, // Begin middle line
    {{+0.0f, -3.0f}, T_CROSS_MEASUREMENT},
    {{-3.9f, -1.1f}, L_CROSS_MEASUREMENT},// Begin penalty boxes
    {{-3.9f, +1.1f}, L_CROSS_MEASUREMENT},
    {{+3.9f, -1.1f}, L_CROSS_MEASUREMENT},
    {{+3.9f, +1.1f}, L_CROSS_MEASUREMENT},
    {{+4.5f, +1.1f}, T_CROSS_MEASUREMENT},
    {{+4.5f, -1.1f}, T_CROSS_MEASUREMENT},
    {{-4.5f, +1.1f}, T_CROSS_MEASUREMENT},
    {{-4.5f, -1.1f}, T_CROSS_MEASUREMENT}
};

static constexpr size_t trials = 1;

static constexpr float min_true_positive_rate = 0.8;

static constexpr float mean_line_separation = 0.1/*m*/;
static constexpr float mean_line_width = 0.05/*m*/;
static constexpr float var_line_width = 0.01/*m*/;

// maybe we need this in a global log class
struct LogData {
    int stamp;
    std::vector<Robot> measurements;
    Robot wcs;
    std::vector<Robot> hypos;
    std::vector<VisionResult> visionResults;
    //.... not nice
};

vector<LogData> log_read(const string &fn) {
    std::ifstream input(fn);
    std::vector<LogData> data;

    LogData ld;
    for (std::string line; getline(input, line);) {
        if (line.size() < 3) {
            continue;
        }

        if (line.find("(") != 0) {
            continue;
        }

        int stamp = atoi(line.substr(1, line.find(")")).c_str());
        ld.stamp = stamp;

        std::string s = line.substr(line.find(")")+2);

        if (s.find("enter new cognition step:") == 0) {
            int step = atoi(s.substr(s.find(": ")+2).c_str());
            // add as new log data element
            data.push_back(ld);
            ld = LogData();
            continue;
        }

        if (s.find("WCS: ") != string::npos) {
            ld.wcs.setFromString(s.substr(5));
            continue;
        }
        if (s.find("Measurement: ") != string::npos) {
            Robot r;
            r.setFromString(s.substr(13));
            ld.measurements.push_back(r);
            continue;
        }
        if (s.find("VisionResult: ") != string::npos) {
            VisionResult vr(s.substr(14));
            ld.visionResults.push_back(vr);
            continue;
        }
    }

    return data;
}

int main(int argc, char **argv) {
    // logger init
    XLogger::quick_init(LOGID);
    LOG_INFO << TEXT_HEADLINE
             << "---------------------------------------------------------";

    LOG_INFO << "hi, this is the line intersection testbox" << TEXT_NORMAL;

    // playingfield
    PlayingField *field = new PlayingField(FieldSize::SPL);

    if (argc < 2) {
        LOG_ERROR << "please provide a data file to test data";
    } else {
        ofstream out("crossings.csv");

        // include all meas with identical stamp
        int last_stamp = -1;
        string last_key = "";

        string data_fn = std::string(argv[1]);

        vector<LogData> dataset = log_read(data_fn.c_str());

        LOG_INFO << "read " << dataset.size() << " data entries";

        int cognition_step = 0;
        vector<LogData>::iterator it;
        vector<VisionResult>::iterator vr;
        for (it = dataset.begin(); it != dataset.end(); ++it) {
            out << "(" << it->stamp << ")" << " enter new cognition step: "
                << cognition_step << endl;

            // just write back current wcs position / vision results
            out << "(" << it->stamp << ")" << " WCS: " << it->wcs << endl;
            for (vr = it->visionResults.begin(); vr != it->visionResults.end(); ++vr) {
                out << "(" << it->stamp << ") VisionResult: " << *vr << endl;

            }

            cognition_step++;
        }

    }

    return 0;
}

