#pragma once
#include <cstdint>

#include "base_detector.h"
#include <framework/image/camimage.h>

#include <HTWKVision/htwk_vision.h>
#include <stdlib.h>

#include "lines.h"

class VisionResult;

namespace bbvision {

#define C_CROSS 0
#define T_CROSS 1
#define L_CROSS 2
#define C_POINT 3

struct Crossing {
    size_t type;
    float orientation;
    float px1;
    float py1;
    float px2;
    float py2;
    float rcs_x1;
    float rcs_y1;
    float rcs_alpha;
    float rcs_distance;
    bool has_rcs = false;
    line_t line1;
    line_t line2;
};

struct CenterCirclePoint {
    size_t type{C_POINT};
    float px1{0};
    float py1{0};
    bool has_rcs{false};
    bool found{false};
};

class CrossingDetector : protected BaseDetector {
public:
    CrossingDetector(int _width, int _height, int8_t *_lutCb, int8_t *_lutCr):
        BaseDetector(_width, _height, _lutCb, _lutCr) {};

    void proceed(CamImage& img,
                 const std::vector<htwk::LineGroup> &linegroups,
                 std::vector<VisionResult>& lineVrs,
                 htwk::color lineColor, htwk::RansacEllipseFitter *ellipseFitter,
                 float center_circle_radius) __attribute__((nonnull));

    float calculateOrientation(Coord intersect, std::vector<Coord> other_ends);

    inline std::vector<Crossing> getTCrossings() const {
        return t_crossings;
    }

    inline std::vector<Crossing> getLCrossings() const {
        return l_crossings;
    }

    inline std::vector<Crossing> getCenterFieldCrossings() const {
        return c_crossings;
    }

    inline CenterCirclePoint getCenterCirclePoint() const {
      return centerCirclePoint;
    }


private:
    std::vector<Crossing> t_crossings;
    std::vector<Crossing> l_crossings;
    std::vector<Crossing> c_crossings;
    CenterCirclePoint centerCirclePoint;

    void findTandLCrossings(CamImage& img, const std::vector<htwk::LineGroup> &lines,
                            const std::vector<VisionResult> &lineVrs,
                            htwk::color lineColor);

    void findCenterCircle(CamImage& img,
                            htwk::RansacEllipseFitter *ellipseFitter,
                            const std::vector<htwk::LineGroup> &lines,
                            float center_circle_radius);

    bool hasGreenBetween(uint8_t *img, htwk::color green, line_t line,
                         Coord point);
    Coord expandPoint(uint8_t *img, htwk::color lineColor, StraightLineEq line_eq,
                      int x1, int x2);
    static line_t expandLine(line_t line, float elongation);
    static int getAcceptancePixelRadius(int px, int py, int camera);
    float getAngleOfIntersectionICS(Coord intersect, line_t line, line_t line2);
    static float getAngleOfIntersectionRCS(Coord line1_P1, Coord line1_P2, Coord line2_P1, Coord line2_P2);
};

} //namespace bbvision
