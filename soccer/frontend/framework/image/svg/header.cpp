#include "header.h"
#include "scale.h"

#include <sstream>
#include <iomanip>

using namespace std;

void SVGHeader::setViewBox(float x, float y, float w, float h) {
    viewBoxX = to_mm(x);
    viewBoxY = to_mm(y);
    viewBoxWidth = to_mm(w);
    viewBoxHeight = to_mm(h);
}

void SVGHeader::setSize(float w, float h) {
    width = to_mm(w);
    height = to_mm(h);
}

string SVGHeader::str() const {

    stringstream out;
    out << std::fixed << std::setprecision(0);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
        << "viewBox=\"" << viewBoxX << " "
        << viewBoxY << " "
        << viewBoxWidth << " "
        << viewBoxHeight << "\" "
        << ">"
        << endl;

    return out.str();
}

// vim: set ts=4 sw=4 sts=4 expandtab:
