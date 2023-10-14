#include "body.h"
#include "scale.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <string_view>

#include <framework/util/assert.h>

void SVGBody::addRectangle(float x0, float y0, float w, float h, std::string_view color, std::string_view id) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);

    ss << "<rect ";
    if (!id.empty())
        ss << "id=\"" << id << "\" ";
    
    ss << "x=\"" << to_mm(x0)
       << "\" y=\"" << to_mm(y0)
       << "\" width=\"" << to_mm(w)
       << "\" height=\"" << to_mm(h)
       << "\" fill=\"" << color
       << "\" />";

    _primitives.push_back(ss.str());
}

void SVGBody::addLine(
        float x0, float y0, float x1, float y1, float strokeWidth, std::string_view color, std::string_view id) {
    /* <line x1="50" y1="50" x2="200" y2="200" stroke="blue" stroke-width="4" /> */

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);

    ss << "<line ";
    if (!id.empty())
        ss << "id=\"" << id << "\" ";

    ss << "x1=\"" << to_mm(x0) << "\""
       << "y1=\"" << to_mm(y0) << "\""
       << "x2=\"" << to_mm(x1) << "\""
       << "y2=\"" << to_mm(y1) << "\""
       << "stroke=\"" << color << "\""
       << "stroke-width=\"" << strokeWidth << "\""
       << " />";

    _primitives.push_back(ss.str());
}

void SVGBody::addCircle(
        float x, float y, float radius, float strokeWidth, std::string_view color, std::string_view id, std::string_view fill) {

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);

    ss << "<circle ";
    if (!id.empty())
        ss << "id=\"" << id << "\" ";

    ss << "cx=\"" << to_mm(x)
       << "\" cy=\"" << to_mm(y)
       << "\" r=\"" << to_mm(radius)
       << "\" stroke=\"" << color
       << "\" stroke-width=\"" << to_mm(strokeWidth)
       << "\" fill=\"" << fill << "\"/>";

    _primitives.push_back(ss.str());
}

void SVGBody::addPolyline(
        const std::vector<float> &points, float strokeWidth, std::string_view color, std::string_view id) {

    jsassert(points.size() % 2 == 0);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);

    ss << "<polyline ";
    if (!id.empty())
        ss << "id=\"" << id << "\" ";
    
    ss << "points=\"";
    for (size_t i = 0; i < points.size()-2; i += 2) {
        float x = points[i];
        float y = points[i+1];
        ss << to_mm(x) << "," << to_mm(y) << " ";
    }

    float x = points[points.size()-2];
    float y = points[points.size()-1];

    ss << to_mm(x) << "," << to_mm(y) << "\" "
       << "stroke=\"" << color << "\" "
       << "stroke-width=\"" << to_mm(strokeWidth) << "\" "
       << "fill=\"none\"/>";

    _primitives.push_back(ss.str());
}

void SVGBody::addText(float x, float y, std::string_view text, std::string_view color, std::string_view id) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);

    ss << "<text ";
    if (!id.empty())
        ss << "id=\"" << id << "\" ";
    
    ss << "x=\"" << to_mm(x)
       << "\" y=\"" << to_mm(y)
       << "\" style=\"font: bold 300px monospace;";

    if (!color.empty())
        ss << " fill: " << color << ";";

    ss << "\" transform=\"translate(-69 111)\">" // center text
       << text << "</text>";

    _primitives.push_back(ss.str());
}


std::string SVGBody::str() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);

    for (const std::string &obj : _primitives) {
        ss << obj << std::endl;
    }

    return ss.str();
}

// vim: set ts=4 sw=4 sts=4 expandtab:
