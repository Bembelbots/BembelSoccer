#pragma once

#include "playingfield.h"

class SVGImage;
class SVGBody;

/**
 * Draws a playingfield into a svg.
 */
class PlayingfieldPainter {

public:


    /**
     * Create a new painter that draws on the provided image
     * and sets the correct image size.
     */
    PlayingfieldPainter(SVGImage &, const PlayingField &);

    /**
     * Create a new painter that draws on the provided image-body.
     */
    PlayingfieldPainter(SVGBody &, const PlayingField &);

    /**
     * Draw the background of the playingfield.
     */
    void drawBackground();

    /**
     * Draw the lines defined on the playingfield.
     */
    void drawLines();

    /**
     * Draw the goal posts.
     */
    void drawPoles();

    /**
     * Draw the crosses defined on the playingfield.
     */
    void drawCrosses();

    /**
     * Draw number for each players ready position
     */
    void drawReadyPositions();

    /**
     * Draw number for each players initial setup position
     */
    void drawInitialPositions();
    void drawChallengePositions();

private:

    static const std::string LINE_COLOR;
    static const std::string BG_COLOR;
    static const std::string CROSS_L_COLOR;
    static const std::string CROSS_T_COLOR;
    static const std::string CROSS_X_COLOR;
    static const std::string POLE_COLOR;

    static constexpr float CROSS_SIZE = 0.25;

    const PlayingField &pf; //< Use the definitions of this playingfield.
    SVGBody &svg; //< Draw on this image.

    void drawLandmark(LandmarkLine);
    void drawLandmark(LandmarkCenterCircle);
    void drawLandmark(LandmarkCross);
    void drawLandmark(LandmarkPole);

    /**
     * Swaps the end points of line, so that
     * - the upper point is first for vertical lines
     * - the left point is the first one for horizontal lines.
     */
    LandmarkLine fixLineOrientation(LandmarkLine);

    /**
     * Transpose all points on a polyline by (tx, ty).
     */
    void transposePolyline(std::vector<float> &, float tx, float ty);

    /**
     * Rotate all points on a polyline by alpha.
     */
    void rotatePolyline(std::vector<float> &, float alpha);

};

// vim: set ts=4 sw=4 sts=4 expandtab:
