#pragma once

#include "header.h"
#include "body.h"

#include <string>
#include <vector>

/**
 * Generate a svg-image from graphic primitives.
 * Keep in mind, the coord system of SVG looks like this:
 * Origin is in the top left corner, with x pointing right and y-axis pointing
 * down.
 *
 * (0,0)
 *   ---> x
 *   |
 *   v
 *   y
 */
class SVGImage {
public:

    SVGHeader header;
    SVGBody body;

    void saveToFile(const std::string &fname) const;

    /**
     * Generate the entire svg from header and body.
     */
    std::string str() const;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
