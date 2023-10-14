#pragma once

#include <string>

/**
 * Header of a svg-image. Contains the size of the image and the viewbox.
 */
class SVGHeader {

public:

    /**
     * Set the viewbox of the svg image with (x, y) being the top left corner
     * of the box.
     *
     * The viewbox is the visible part of the image.
     *
     * If X marks the origin then a svg with viewbox (-5, -3, 10, 6) would
     * look like this:
     *  ---------
     * |         |
     * |    X    |
     * |         |
     *  ---------
     */
    void setViewBox(float x, float y, float w, float h);

    /**
     * Set the size of the image.
     */
    void setSize(float w, float h);

    /**
     * Generate xml header.
     */
    std::string str() const;


private:

    float viewBoxX{0};
    float viewBoxY{0};
    float viewBoxWidth{0};
    float viewBoxHeight{0};

    float width{0};
    float height{0};

};

// vim: set ts=4 sw=4 sts=4 expandtab:
