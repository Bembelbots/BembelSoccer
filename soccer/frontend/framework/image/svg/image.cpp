#include "image.h"

#include <sstream>
#include <fstream>

using namespace std;

string SVGImage::str() const {
    stringstream ss;

    string headerxml = header.str();
    string bodyxml   = body.str();

    ss << headerxml << bodyxml << "</svg>";

    return ss.str();
}



void SVGImage::saveToFile(const string &fname) const {
    std::ofstream out(fname);

    out << str();

    out.close();
}
