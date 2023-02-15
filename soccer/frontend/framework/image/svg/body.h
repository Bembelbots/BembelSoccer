#pragma once

#include <vector>
#include <string_view>

/**
 * Body of a svg-image. Contains all primitives that should be drawn.
 */
class SVGBody {

public:
    /**
     * Add a rectangle with upper left corner (x0, y0).
     */
    void addRectangle(float x0, float y0, float w, float h, std::string_view color, std::string_view id = "");

    /**
     * Draw a line from (x0, y0) to (x1, y1) with width strokeWidth.
     */
    void addLine(float x0, float y0, float x1, float y1, float strokeWidth, std::string_view color,
            std::string_view id = "");

    /**
     * Draw a circle with center pofloat (x, y) and line width `strokeWidth`.
     */
    void addCircle(float x, float y, float radius, float strokeWidth, std::string_view color, std::string_view id = "", std::string_view fill = "none");

    /**
     * Draw a polygon line from a list of points with line width `strokeWidth`.
     *
     * Points is a vector where to subsequent entries represent a single point
     * of the polyline.
     * So the polyline [(x1, y1), (x2, y2), (x3, y3)] can be drawn with
     * the vector {x1, y1, x2, y2, x3, y3}.
     */
    void addPolyline(
            const std::vector<float> &points, float strokeWidth, std::string_view color, std::string_view id = "");

    /**
     * Draw text at specified location.
     */
    void addText(float x, float y, std::string_view text, std::string_view color = "", std::string_view id = "");

    std::string str() const;

private:

    std::vector<std::string> _primitives; ///< Save the primitves as xml objects

};

// vim: set ts=4 sw=4 sts=4 expandtab:
