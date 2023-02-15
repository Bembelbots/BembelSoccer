#pragma once

#include <string>
#include <sstream>
#include <iomanip>

/* convert any given parameter to a string
 * @val any parameter, which has a string representation
 * @precision (default=0), restricts the precision for i.e., floats
 * @return the converted string
 */
template <class T>
std::string str(const T &val, int precision = 0) {
    std::stringstream inp1;
    if (precision == 0) {
        inp1 << val;
    } else {
        inp1 << std::fixed << std::setprecision(precision) << val;
    }
    return inp1.str();
}

// vim: set ts=4 sw=4 sts=4 expandtab:
