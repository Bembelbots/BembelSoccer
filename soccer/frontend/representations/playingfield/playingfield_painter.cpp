#include "playingfield_painter.h"

#include <framework/util/assert.h>
#include <framework/image/svg/image.h>
#include <representations/bembelbots/constants.h>

#include <string>
#include <string_view>

using namespace std;

const string PlayingfieldPainter::LINE_COLOR = "white";
const string PlayingfieldPainter::BG_COLOR = "green";
const string PlayingfieldPainter::CROSS_L_COLOR = "blue";
const string PlayingfieldPainter::CROSS_T_COLOR = "blue";
const string PlayingfieldPainter::CROSS_X_COLOR = "blue";
const string PlayingfieldPainter::POLE_COLOR = "lightgray";

PlayingfieldPainter::PlayingfieldPainter(SVGImage &img, const PlayingField &field)
  : pf(field)
  , svg(img.body) {
    img.header.setViewBox(-pf._length / 2.f, -pf._width / 2.f, pf._length, pf._width);
}

PlayingfieldPainter::PlayingfieldPainter(SVGBody &img, const PlayingField &field)
  : pf(field)
  , svg(img) {}

void PlayingfieldPainter::drawLines() {

    vector<LandmarkLine> lines = pf.getLines();

    for (const LandmarkLine &line : lines) {
        drawLandmark(line);
    }

    drawLandmark(pf._circle);
}

void PlayingfieldPainter::drawPoles() {

    vector<LandmarkPole> poles = pf.getPoles();

    for (const LandmarkPole &pole : poles) {
        drawLandmark(pole);
    }
}


void PlayingfieldPainter::drawCrosses() {

    for (const LandmarkCross &cross : pf._crosses) {
        drawLandmark(cross);
    }
}

void PlayingfieldPainter::drawReadyPositions(const bool &seven) {
    const int players = (seven)?NUM_PLAYERS:5;
    for (int i = 0; i < players; ++i) {
        std::string num = std::to_string(i + 1);

        auto p = pf.getReadyPose(i, false);
        svg.addText(p.coord.x, p.coord.y, num, "gray", "Ready_" + num);

        p = pf.getReadyPose(i, true);
        svg.addText(p.coord.x, p.coord.y, num, "navy", "Ready_Kickoff_" + num);
    }
}

void PlayingfieldPainter::drawInitialPositions(const bool &seven) {
    const int players = (seven)?NUM_PLAYERS:5;
    const std::string color = (seven)?"silver":"black";
    for (int i = 0; i < players; ++i) {
        vector <DirectedCoord> poses = pf.getInitialPose(i, RobotRole::NONE, seven);
        std::string num = std::to_string(i+1);
        for (auto p: poses)
            svg.addText(p.coord.x, p.coord.y, num, color, "Initial_" + num);
    }
}

void PlayingfieldPainter::drawChallengePositions() {
    drawInitialPositions(true);
    drawReadyPositions(true);
}

void PlayingfieldPainter::drawLandmark(LandmarkLine line) {

    line = fixLineOrientation(line);

    float offset = pf._lineWidth / 2;

    float x0 = line.start_x - offset;
    float y0 = line.start_y + offset;

    float x1 = line.end_x + offset;
    float y1 = line.end_y - offset;

    float width = x1 - x0;
    float height = y0 - y1;

    // svg only accepts non-negative height and width
    jsassert(width >= 0);
    jsassert(height >= 0);

    if (line.name == Line::OWN_PENALTY_SHOOTMARK || line.name == Line::OPP_PENALTY_SHOOTMARK) {
        std::string name{enumToStr(line.name)};
        svg.addRectangle(x0-width/2.f, -y0, width*2, height, LINE_COLOR, name + "0");
        svg.addRectangle(x0, -y0-width/2.f, width, height*2, LINE_COLOR, name + "1");
    } else {
        svg.addRectangle(x0, -y0, width, height, LINE_COLOR, enumToStr(line.name));
    }
}

void PlayingfieldPainter::drawLandmark(LandmarkCenterCircle centerCircle) {

    float radius = centerCircle.wcs_radius;

    float cx = centerCircle.wcs_x;
    float cy = -centerCircle.wcs_y;

    float width = pf._lineWidth;

    svg.addRectangle(-width, -width/2, width*2, width, LINE_COLOR, "KICKOFF_SPOT");
    svg.addCircle(cx, cy, radius, width, LINE_COLOR, enumToStr(centerCircle.name));
}

void PlayingfieldPainter::drawLandmark(LandmarkCross cross) {

    vector<float> points;
    std::string color;

    switch (cross.degree) {
        case 2:
            points = {0, CROSS_SIZE, 0, 0, CROSS_SIZE, 0};
            color = CROSS_L_COLOR;
            break;
        case 3:
            points = {0, CROSS_SIZE, 0, -CROSS_SIZE, 0, 0, CROSS_SIZE, 0};
            color = CROSS_T_COLOR;
            break;
        case 4:
            points = {0, CROSS_SIZE, 0, -CROSS_SIZE, 0, 0, CROSS_SIZE, 0, -CROSS_SIZE, 0};
            color = CROSS_X_COLOR;
            break;
        default:
            JS_UNREACHABLE();
    }

    rotatePolyline(points, cross.wcs_alpha);
    transposePolyline(points, cross.wcs_x, cross.wcs_y);

    //Fix y axis.
    for (size_t i = 1; i < points.size(); i += 2) {
        points[i] *= -1;
    }

    svg.addPolyline(points, pf._lineWidth / 2, CROSS_L_COLOR, enumToStr(cross.name));
}

void PlayingfieldPainter::drawLandmark(LandmarkPole pole) {
    svg.addCircle(pole.wcs_x, -pole.wcs_y, pole.wcs_width/2.f, 0, POLE_COLOR, enumToStr(pole.name), POLE_COLOR);
}

void PlayingfieldPainter::transposePolyline(vector<float> &points, float tx, float ty) {
    for (size_t i = 0; i < points.size(); i += 2) {
        points[i] += tx;
        points[i + 1] += ty;
    }
}

void PlayingfieldPainter::rotatePolyline(vector<float> &points, float alpha) {
    float r11 = cosf(alpha);
    float r12 = -sinf(alpha);
    float r21 = sinf(alpha);
    float r22 = r11;

    for (size_t i = 0; i < points.size(); i += 2) {
        float x = points[i];
        float y = points[i + 1];
        points[i] = r11 * x + r12 * y;
        points[i + 1] = r21 * x + r22 * y;
    }
}

LandmarkLine PlayingfieldPainter::fixLineOrientation(LandmarkLine line) {
    if (line.start_x == line.end_x) {

        if (line.start_y >= line.end_y) {
            return line;
        }

        float tmp_y = line.start_y;
        line.start_y = line.end_y;
        line.end_y = tmp_y;
        return line;
    }

    if (line.start_x <= line.end_x) {
        return line;
    }

    float tmp_x = line.start_x;
    line.start_x = line.end_x;
    line.end_x = tmp_x;
    return line;
}

void PlayingfieldPainter::drawBackground() {
    float x0 = -pf._length / 2.f;
    float y0 = pf._width / 2.f;

    svg.addRectangle(x0, -y0, pf._length, pf._width, BG_COLOR);
}

// vim: set ts=4 sw=4 sts=4 expandtab:
