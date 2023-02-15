#include "old.h"

#include <boost/math/special_functions/fpclassify.hpp>

#include <vector>

float lineToPointDist(const std::vector<float> &point, const std::vector<float> &line) {
    return (point[0] * line[0] + point[1] * line[1] + line[2]) /
           ((fabsf(line[2]) / -line[2]) * hypotf(line[0], line[1]));
}

std::vector<float> vectorCrossProduct(const std::vector<float> &a, const std::vector<float> &b) {

    return normVector(vectorCrossProductUnNormed(a, b));
}

std::vector<float> vectorCrossProductUnNormed(const std::vector<float> &a, const std::vector<float> &b) {
    return std::vector<float>{a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

std::vector<float> getHesseNormalFormOfLine(const std::vector<float> &lineIn) {
    std::vector<float> line(lineIn);

    float sign = (fabsf(line[2]) / line[2]);
    if (boost::math::isnan(sign)) {
        sign = 1.0f;
    }
    float hnf = sign * hypotf(line[0], line[1]);
    line[0] /= hnf;
    line[1] /= hnf;
    line[2] /= hnf;
    return line;
}

std::vector<float> normVector(const std::vector<float> &vecIn) {
    std::vector<float> vec(vecIn);
    // jsassert(3 == vec.size());
    if (0.0f < fabsf(vec[2])) {
        vec[0] /= vec[2];
        vec[1] /= vec[2];
        vec[2] /= vec[2];
        return vec;
    } else {
        return vec;
    }
}

Measurement1D::Measurement1D() : x(0.0f), confidence(0.0f), localConfidence(0.0f) {
}

Measurement1D::Measurement1D(float x, float confidence) : x(x), confidence(confidence), localConfidence(0.0f) {
}

Measurement2D::Measurement2D() : x(0.0f), y(0.0f), confidence(0.0f), timestamp(0) {
}

Measurement2D::Measurement2D(float x, float y, float confidence, TimestampMs ts)
  : x(x), y(y), confidence(confidence), timestamp(ts) {
}

/*
 * calculate motion vector of two measurements
 * problem: what is the confidence of the difference, and which ts to use?
 * now: 1, since the difference is purely sure!
 * timestamp, difference between both measurements
 */
Measurement2D Measurement2D::operator-(Measurement2D const &rhs) const {
    Measurement2D ret;
    ret.x = x - rhs.x;
    ret.y = y - rhs.y;
    ret.confidence = 1.0f;
    ret.timestamp = timestamp - rhs.timestamp;
    if (ret.timestamp < 0) {
        ret.timestamp = 0 - ret.timestamp;
    }
    return ret;
}

////////
////////
////////
////////
////////
////////
////////
////////
//// new stuff starts here using weird, fancym swift,  new features like:
// -> code reuse! -> class-freaking-objects -> abstraction -> RELIABILITY
////////
