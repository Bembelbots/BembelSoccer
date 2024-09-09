#pragma once

#include <vector>
#include <limits>
#include <cmath>

#include "coord.h"
#include "constants.h"
#include "fast_math.h"
#include "../util/clock.h"

/**
* calculates the distance from a point to a line
* all in homogenius coordinates
*/
float lineToPointDist(const std::vector<float> &point, const std::vector<float> &line);

/// PART OF line/circle
/////// does some math, why? and even more, why is this not related to a class
//////// <<------ THIS CAN ALSO BE PART OF class Line....!!!!
// - pose/playingfield.cpp ->
//   'line.equation = getHesseNormalFormOfLine(line.equationNormal);'
// - <no other uses, so remove usage in playingfield (if appropriate) and
//   fully remove func here!
/**
* returns a line in HesseNormalForm
* @param vector line == size 3 (a,b,c)^T <==> a*x+b*y+c=0
* @return vector line == size 3 1/||n||*(a,b,c)^T, n=sqrtf(a^2+b^2)
*/
std::vector<float> getHesseNormalFormOfLine(std::vector<float> line);

//// The 6 above either to Line or Circle.... see definition-comment below
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////// these BELOW are needed, somehow ...
//  maybe these fit inside a NumericVector class, but they are not used often
//  enough to cover the expenses of writing one, thus.... put them in a separate
//  - VectorOperations namespace or something like that ....
//  - make a (fake)class realizing all as static inside it (notation-wise equal)
//
//////// needed in measurementtoolbox .....
//////// cross product of vectors seem to be needed, but isn't this
//////// provided by the cv::mat stuff ????
// - this can not easily be removed: measuretoolsbox, playingfield + cornerdetect
/**
* calculates the vec crossproduct for a 3d vector
* in this case a 2d homogenius vector
* normed, so that the 3rd componet is 1
* @param vector a == size 3
* @param vector b ||
* @return vector
*/
std::vector<float> vectorCrossProduct(const std::vector<float> &a, const std::vector<float> &b);

/////// does some math, why? and even more, why is this not related to a class
// - pose/playingfield.cpp ->
//   'line.equationNormal = vectorCrossProductUnNormed(startP, endP);'
// - core/util/mathtoolbox.cpp ->
//   'return normVector(vectorCrossProductUnNormed(a, b));'
// - <no other uses, so remove usage in playingfield (if appropriate) and
//   fully remove func here!
/**
* calculates the vec crossproduct for a 3d vector
* in this case a 2d homogenius vector
* @param vector a == size 3
* @param vector b ||
* @return vector
*/
std::vector<float> vectorCrossProductUnNormed(const std::vector<float> &a, const std::vector<float> &b);

/////// normVector is also an UNUSED [;)] method in vision/visinotoolbox....
// - vision/toolbox/visiontoolbox.h (commented out now!)
// - <no other uses, so -> remove it!>
/**
* normalizes a vector in homogenius coords
* to the scalingfactor 1
* @param vector size3
* @return vector size3, 3rd elemet = 1
*/
std::vector<float> normVector(std::vector<float> vec);

class Measurement1D {
public:
    Measurement1D();
    Measurement1D(float x, float confidence);

    float x;
    float confidence;
    float localConfidence;
};

class Measurement2D {
public:
    Measurement2D();
    Measurement2D(float x, float y, float confidence, TimestampMs ts = 0);
    Measurement2D operator-(Measurement2D const &rhs) const;
    float x;
    float y;
    float confidence;
    TimestampMs timestamp;
};

// TODO: Replace by proper line definition
using line_t = std::pair<Coord, Coord>;
