#include "crossing_detector.h"
#include <boost/math/constants/constants.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include <cstring>
#include <cmath>
#include <framework/logger/logger.h>
#include <representations/bembelbots/constants.h>
#include <framework/util/assert.h>
#include <framework/image/camimage.h>
#include <representations/vision/visiondefinitions.h>
#include <algorithm>


namespace bbvision {

static constexpr float M_TWO_PI_F{(2 * M_PI_F)};
static constexpr float MIN_INTERSECTION_ANGLE{(10_deg).v}; //~10 deg
static constexpr float MAX_DIST_LINE_TO_CROSSING{0.15f}; // allow for small gap between the detected line & crossing
static constexpr float MIN_DIST_POINT_TO_TCROSSING{0.05f}; // allow only small gap between crossing and closest point in T crossing

inline float transformAxes(float slope, float x) {
    return abs(x - slope * x);
}

inline bool isOnLine(StraightLineEq eq, line_t line, Coord point) {
    float start = transformAxes(eq.gradient, line.first.x);
    float end = transformAxes(eq.gradient, line.second.x);
    float point_new = transformAxes(eq.gradient, point.x);

    return point_new > start && point_new < end;
}

inline line_t getLine(htwk::LineEdge line) {
    return {{line.px1, line.py1}, {line.px2, line.py2}};
}

inline line_t getHiddenLine(line_t line1, line_t line2) {
    return {{(line1.first.x + line2.first.x) / 2, (line1.first.y + line2.first.y) / 2},
        {(line1.second.x + line2.second.x) / 2, (line1.second.y + line2.second.y) / 2}};
}

inline std::string getKey(line_t line) {
    std::stringstream ss;
    ss << line.first.x << "," << line.first.y;
    ss << ";" << line.second.x << "," << line.second.y;
    return ss.str();
}

inline bool isPointOnLineSegment(line_t line, Coord& intersect, float eps=0.1f){
    float fullDist = line.first.dist(line.second);
    float distToIntersect = line.first.dist(intersect);
    float distToIntersect2= line.second.dist(intersect);
    float minPixelDistance = 3.f; // min difference between intersection and line end point
    if(distToIntersect < minPixelDistance){ // Check if intersection is very close to linesegment endpoint
        // Point is to close thus do not cosinder 'real' onLineSegment
        return false;
    } 
    if(distToIntersect2 < minPixelDistance){
        // Point is to close thus do not cosinder 'real' onLineSegment
        return false;
    }
    if(fullDist - (distToIntersect + distToIntersect2) < 0-eps){
        return false;
    }
    return true;
}

inline bool getLineIntersection(line_t line1, line_t line2, Coord& intersection){
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = line1.second.x - line1.first.x;
    s1_y = line1.second.y - line1.first.y;
    s2_x = line2.second.x - line2.first.x;
    s2_y = line2.second.y - line2.first.y;

    float s, t;
    s = (-s1_y * (line1.first.x-line2.first.x) + s1_x * (line1.first.y-line2.first.y)) /
        (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (line1.first.y-line2.first.y) - s2_y * (line1.first.x-line2.first.x)) /
        (-s2_x * s1_y + s1_x * s2_y);

    if(s >= 0 && s<= 1 && t>= 0 && t<= 1){ // collision
        intersection.x = line1.first.x + (t*s1_x);
        intersection.y = line1.first.y + (t*s1_y);
        return true;
    }
    return false;
}

void CrossingDetector::proceed(CamImage& img,
                               const std::vector<htwk::LineGroup> &linegroups,
                               std::vector<VisionResult>& lineVrs,
                               htwk::color lineColor, htwk::RansacEllipseFitter *ellipseFitter,
                               float center_circle_radius) {
    findTandLCrossings(img, linegroups, lineVrs, lineColor);
    //findCenterFieldCrossings(img, ellipseFitter, linegroups, crossings,
    //                         center_circle_radius);
    findCenterCircle(img, ellipseFitter, linegroups, center_circle_radius);
}

void CrossingDetector::findTandLCrossings(CamImage& img,
        const std::vector<htwk::LineGroup> &linegroups,
        const std::vector<VisionResult> &lineVrs,
        htwk::color lineColor) {

    t_crossings.clear();
    l_crossings.clear();
    
    float elongationFactor = 50; // FIXME: this should be a parameter (define?)

    std::map<std::pair<std::string, std::string>, bool> usedLines;

    // First step: Determine crossing points in ICS

    for(auto lineVr : lineVrs){

        // Wrap vision result line back to line_t type
        line_t hidden_ics{{lineVr.ics_x1, lineVr.ics_y1}, {lineVr.ics_x2, lineVr.ics_y2}};
        // LOG_DEBUG << "\n line 1: \n first point ics: " << hidden_ics.first
        //           << "\n second point ics:  " << hidden_ics.second;

        // Expand line to check for intersecting line segments
        line_t hidden_ics_expand = expandLine(hidden_ics, elongationFactor);

        // Get key for current line
        std::string hidden_key = getKey(hidden_ics);

        for(auto lineVr2 : lineVrs){
            line_t hidden2_ics{{lineVr2.ics_x1, lineVr2.ics_y1}, {lineVr2.ics_x2, lineVr2.ics_y2}};
            // LOG_DEBUG << "line 2: \n first point ics:" << hidden2_ics.first
            //           << "\n second point ics: " << hidden2_ics.second;
            line_t hidden2_ics_expand = expandLine(hidden2_ics, elongationFactor);

            // Get key for other line
            std::string hidden2_key = getKey(hidden2_ics);

            // Check map of used key paris
            if (usedLines.count({hidden2_key, hidden_key}) > 0) {
                continue;
            }
            // Append this line pair to usedLine pair map
            usedLines[{hidden_key, hidden2_key}] = 0;

            // Init intersection coordinate
            Coord intersect_ics{0,0};

            // Check if there is an intersection
            bool isIntersecting = getLineIntersection(hidden_ics_expand, hidden2_ics_expand,
                                                intersect_ics);
            // LOG_DEBUG << "isIntersecting: " << isIntersecting;

            if(!isIntersecting){ // go to next if no intersection
                continue;
            }

            if(intersect_ics.x < 0 || intersect_ics.y < 0 || intersect_ics.x >= img.width || intersect_ics.y >= img.height){
                continue;
            }

            //Second step: Convert all lines and crossings to RCS

            //Initialize intersect_rcs
            Coord intersect_rcs = {0, 0};

            //get rcs position of intersect from ics
            intersect_rcs = img.getRcsPosition(intersect_ics.x, intersect_ics.y);
            //LOG_DEBUG << "intersect_rcs: " << intersect_rcs;

            //get lines in rcs
            line_t hidden_rcs{{lineVr.rcs_x1, lineVr.rcs_y1}, {lineVr.rcs_x2, lineVr.rcs_y2}};
            // LOG_DEBUG << "hidden rcs: " << hidden_rcs.first << ", " << hidden_rcs.second;

            line_t hidden2_rcs{{lineVr2.rcs_x1, lineVr2.rcs_y1}, {lineVr2.rcs_x2, lineVr2.rcs_y2}};
            // LOG_DEBUG << "hidden2 rcs: " << hidden2_rcs.first << ", " << hidden2_rcs.second;

            //Third step: Check crossings in RCS

            // Define points on the two lines

            Coord hidden_rcs_P1{lineVr.rcs_x1, lineVr.rcs_y1};
            // LOG_DEBUG<<"lineVr.rcs_x1:"<< lineVr.rcs_x1;
            // LOG_DEBUG<<"lineVr.rcs_y1:"<< lineVr.rcs_y1;
            // LOG_DEBUG<<"hidden_rcs_P1: "<<hidden_rcs_P1.x;
            Coord hidden_rcs_P2{lineVr.rcs_x2, lineVr.rcs_y2};
            // LOG_DEBUG<<"lineVr.rcs_x2:"<< lineVr.rcs_x2;
            // LOG_DEBUG<<"lineVr.rcs_y2:"<< lineVr.rcs_y2;
            Coord hidden2_rcs_P1{lineVr2.rcs_x1, lineVr2.rcs_y1};
            // LOG_DEBUG<<"lineVr2.rcs_x1:"<< lineVr2.rcs_x1;
            // LOG_DEBUG<<"lineVr2.rcs_y1:"<< lineVr2.rcs_y1;
            Coord hidden2_rcs_P2{lineVr2.rcs_x2, lineVr2.rcs_y2};
            // LOG_DEBUG<<"lineVr2.rcs_x2:"<< lineVr2.rcs_x2;
            // LOG_DEBUG<<"lineVr2.rcs_y2:"<< lineVr2.rcs_y2;

            //a. Check crossings using distance between points on lines and crossings.

            //Initialize closer point to the intersection on hidden
            Coord hidden_closerPoint = {0, 0};

            //Initialize further point to the intersection on hidden which will be needed to distinguish L, T and X crossing
            Coord hidden_furtherPoint = {0, 0};

            // Define closer and further point on hidden
            if (hidden_rcs_P2.dist(intersect_rcs) > hidden_rcs_P1.dist(intersect_rcs)) {
                hidden_closerPoint = hidden_rcs_P1;
                hidden_furtherPoint = hidden_rcs_P2;
            } else {
                hidden_closerPoint = hidden_rcs_P2;
                hidden_furtherPoint = hidden_rcs_P1;
            }
            //LOG_DEBUG << "hidden_closerPoint: " << hidden_closerPoint.x << ", " << hidden_closerPoint.y;
            //LOG_DEBUG << "hidden_furtherPoint: " << hidden_furtherPoint.x << ", " << hidden_furtherPoint.y;

            //Initialize closer point to the intersection on hidden2
            Coord hidden2_closerPoint = {0, 0};

            //Initialize further point to the intersection on hidden2 which will be needed to distinguish L, T and X crossing
            Coord hidden2_furtherPoint = {0, 0};

            // Define closer and further point on hidden2
            if (hidden2_rcs_P2.dist(intersect_rcs) > hidden2_rcs_P1.dist(intersect_rcs)) {
                hidden2_closerPoint = hidden2_rcs_P1;
                hidden2_furtherPoint = hidden2_rcs_P2;
            } else {
                hidden2_closerPoint = hidden2_rcs_P2;
                hidden2_furtherPoint = hidden2_rcs_P1;
            }
            //LOG_DEBUG << "hidden2_closerPoint: " << hidden2_closerPoint.x << ", " << hidden2_closerPoint.y;
            //LOG_DEBUG << "hidden2_furtherPoint: " << hidden2_furtherPoint.x << ", " << hidden2_furtherPoint.y;

            // Check that distance between crossing and closer point is less than MAX_DIST_LINE_TO_CROSSING.
            if (std::max(hidden_closerPoint.dist(intersect_rcs), hidden2_closerPoint.dist(intersect_rcs)) > MAX_DIST_LINE_TO_CROSSING) {
                //LOG_DEBUG << "Continued because of distance between crossing and closest point";
                continue;
            }

            //b. Check crossings using angles between the two crossing lines

            //Calculate angles
            float intersectAngleRCS_t =
                    getAngleOfIntersectionRCS(hidden_rcs_P1, hidden_rcs_P2, hidden2_rcs_P1, hidden2_rcs_P2);
            // LOG_DEBUG << "intersectAngleRCS_t = " << intersectAngleRCS_t;

            //Initiate tolerated error in angles
            float maxAngleError = 0;

            //Filter crossing using the smallest y-coordinate in ICS of each line (the further away the point, the less exact the angle)
            //Tolerate 15 degrees angle error if any of the ICS y-coordinates is smaller than 100
            if (lineVr.ics_y1 < 100 || lineVr.ics_y2 < 100 || lineVr2.ics_y1 < 100 || lineVr2.ics_y2 < 100) {
                maxAngleError = 0.261799;
            }
            //Tolerate 8 degrees angle error if any of the ICS y-coordinates is larger or equal to 100 and smaller than 200
            else if ((lineVr.ics_y1 >= 100 && lineVr.ics_y1 < 200) ||
                        (lineVr.ics_y2 >= 100 && lineVr.ics_y2 < 200) ||
                        (lineVr2.ics_y1 >= 100 && lineVr2.ics_y1 < 200) ||
                        (lineVr2.ics_y2 >= 100 && lineVr2.ics_y2 < 200)) {
                maxAngleError = 0.139626;
            }
            //Tolerate 2 degrees angle error if all of the ICS y-coordinates are larger or equal to 200
            else {
                maxAngleError = 0.0349066;
            }

            //LOG_DEBUG<<"maxAngleError: "<<maxAngleError;
            // If the difference between 90 degrees and the calculated angle is larger than maxAngleError, don't take this crossing
            if (1.5708 - intersectAngleRCS_t > maxAngleError) {
                //LOG_DEBUG << "Continued because of maxAngle";
                continue;
            }

            //Fourth step: Find out which types of crossings the found crossings are

            // Define a bool which determines whether the order of points on the line is point - intersection - point for hidden and hidden 2
            bool pointIntersectionPoint_hidden = true;
            if (hidden_furtherPoint.dist(hidden_closerPoint) < hidden_furtherPoint.dist(intersect_rcs)) {
                pointIntersectionPoint_hidden = false;
                // LOG_DEBUG<<"hidden: pointpointIntersection = "<<pointIntersectionPoint_hidden;
            }

            bool pointIntersectionPoint_hidden2 = true;
            if (hidden2_furtherPoint.dist(hidden2_closerPoint) < hidden2_furtherPoint.dist(intersect_rcs)) {
                pointIntersectionPoint_hidden2 = false;
                // LOG_DEBUG<<"hidden2: pointpointIntersection = "<<pointIntersectionPoint_hidden2;
            }

            //Classify L crossings based on bools
            bool isLCrossing = false;
            if ((pointIntersectionPoint_hidden == false) && (pointIntersectionPoint_hidden2 == false)) {
                isLCrossing = true;
                //LOG_DEBUG << "isLCrossing \n";
            }

            //Classify T crossings based on bools
            bool isTCrossing = false;
            if ((pointIntersectionPoint_hidden == true && pointIntersectionPoint_hidden2 == false) ||
                    (pointIntersectionPoint_hidden == false && pointIntersectionPoint_hidden2 == true)) {
                isTCrossing = true;
                //LOG_DEBUG << "isTCrossing \n";
            }

            //Sort out T crossings where the point closer to the intersection on the point-intersection-point axis 
            //is too close to the intersection
            if (isTCrossing == true) {
                if (pointIntersectionPoint_hidden == true){
                //LOG_DEBUG << "dist point intersection in T-crossing hidden: " << hidden_closerPoint.dist(intersect_rcs);
                    if (hidden_closerPoint.dist(intersect_rcs) <= MIN_DIST_POINT_TO_TCROSSING){
                        continue;
                    }
                } 
                else if (pointIntersectionPoint_hidden2 == true){
                //LOG_DEBUG << "dist point intersection in T-crossing hidden2: " << hidden2_closerPoint.dist(intersect_rcs);
                    if (hidden2_closerPoint.dist(intersect_rcs) <= MIN_DIST_POINT_TO_TCROSSING){
                        continue;
                    }
                }
            }
           
            // Init crossings vision result with type
            Crossing crossing{};
            if(isLCrossing){
                crossing.type = L_CROSS;
                crossing.px1 = intersect_ics.x;
                crossing.py1 = intersect_ics.y;
            } else if (isTCrossing) {
                crossing.type = T_CROSS;
                crossing.px1 = intersect_ics.x;
                crossing.py1 = intersect_ics.y;
            }


            // L crossings
            if(crossing.type == L_CROSS){
                // Check which point is closer to intersection to get correct vector orientations

                //Initialize Coordinates for vectors and line lengths
                Coord hidden_vec_RCS{0,0};
                Coord hidden2_vec_RCS{0,0};
                float lineLengthHidden1 = 0.f;
                float lineLengthHidden2 = 0.f;

                //Determine coordinates for vectors and line lengths
                hidden_vec_RCS = hidden_furtherPoint - hidden_closerPoint;
                // LOG_DEBUG << "hidden_vec_RCS: " << hidden_vec_RCS;
                lineLengthHidden1 = intersect_rcs.dist(hidden_furtherPoint);
                // LOG_DEBUG << "lineLengthHidden1 = " << lineLengthHidden1;

                hidden2_vec_RCS = hidden2_furtherPoint - hidden2_closerPoint;
                // LOG_DEBUG << "hidden2_vec_RCS: " << hidden2_vec_RCS;
                lineLengthHidden2 = intersect_rcs.dist(hidden2_furtherPoint);
                // LOG_DEBUG << "lineLengthHidden2 = " << lineLengthHidden2;

                // Get direction angles of above vectors
                Angle dirAng_hidden = hidden_vec_RCS.angle();
                // LOG_DEBUG << "hidden direction angle: " << hidden_vec_RCS.angle();
                Angle dirAng_hidden2 = hidden2_vec_RCS.angle();
                // LOG_DEBUG << "hidden2 direction angle: " << hidden2_vec_RCS.angle();

                // Check which direction angle is the correct one
                if(Angle::normalize((dirAng_hidden - dirAng_hidden2).rad() < 0)){
                    crossing.orientation = dirAng_hidden.rad();
                    // DEBUG
                    crossing.px2 = hidden_ics.first.x;
                    crossing.py2 = hidden_ics.first.y;
                } else{
                    crossing.orientation = dirAng_hidden2.rad();
                    // DEBUG
                    crossing.px2 = hidden2_ics.first.x;
                    crossing.py2 = hidden2_ics.first.y;
                }

                // PushBack vision result
                l_crossings.push_back(crossing);
            }

            // T crossings
            else if (crossing.type == T_CROSS) {
                // Check on which line the intersection was NOT
                // and take the orientation of the respective line
                Coord hidden_vec_RCS{0,0};
                Coord hidden2_vec_RCS{0,0};

                //If point is on hidden, take the orientation of hidden2
                if (pointIntersectionPoint_hidden == true) {
                    hidden2_vec_RCS = hidden2_furtherPoint - hidden2_closerPoint;
                    crossing.orientation = hidden2_vec_RCS.direction().rad();
                    // DEBUG
                    crossing.px2 = hidden2_ics.first.x;
                    crossing.py2 = hidden2_ics.first.y;
                }
                //Else if point is on hidden2, take the orientation of hidden
                else if (pointIntersectionPoint_hidden2 == true) {
                    hidden_vec_RCS = hidden_furtherPoint - hidden_closerPoint;
                    crossing.orientation = hidden_vec_RCS.direction().rad();
                    // DEBUG
                    crossing.px2 = hidden_ics.first.x;
                    crossing.py2 = hidden_ics.first.y;
                }

                // PushBack vision result
                t_crossings.push_back(crossing);
            }
        }
    } 
}

void CrossingDetector::findCenterCircle(CamImage& img,
        htwk::RansacEllipseFitter *ellipseFitter, const std::vector<htwk::LineGroup> &lines,
        float center_circle_radius){

    centerCirclePoint.found = false;
    //htwk::RansacEllipseFitter *raf = _htwk->ellipseFitter;
    htwk::Ellipse ellip = ellipseFitter->getEllipse();

    if (not ellip.found) {
        return;
    }

    htwk::point_2d point;
    point.x=0*ellipseFitter->getEllipse().ta;
    point.y=0*ellipseFitter->getEllipse().tb;
    ellipseFitter->transformPoInv(point,ellipseFitter->getEllipse().trans, ellipseFitter->getEllipse().translation);
    float px=point.x;
    float py=point.y;
    
    centerCirclePoint.found = true,
    centerCirclePoint.type = C_POINT;
    centerCirclePoint.px1 = px;
    centerCirclePoint.py1 = py;
}


int CrossingDetector::getAcceptancePixelRadius(int px, int py, int camera){
    // TODO: Take in camera as parameter and adjust function for bottom cam
    int radius = 0;
    // line equation through point (24, 10), (400,40), with (input py, resulting px distance) 
    if(camera == TOP_CAMERA){
        radius += (int)((15.f / 94.f)*py + (760.f/47.f));
    } else {
        radius += 200;
    }   
    return radius;
}

line_t CrossingDetector::expandLine(line_t line, float elongation){
    // expand line by the vectors (x1,y1)<->(x2,y2)
    line_t elongatedLine;
    std::vector<float> direction{0.f,0.f};
    float dirLen = 0.f;
    // elongate first direction (x1,y1)->(x2,y2)
    direction[0] = line.first.x - line.second.x;
    direction[1] = line.first.y - line.second.y;
    dirLen = sqrtf(direction[0]*direction[0] + direction[1]* direction[1]);
    line.first.x += (direction[0]/dirLen) * elongation;
    line.first.y += (direction[1]/dirLen) * elongation;

    // elongate second direction (x2,y2)->(x1,x2)
    direction[0] = line.second.x - line.first.x;
    direction[1] = line.second.y - line.first.y;
    line.second.x += (direction[0]/dirLen) * elongation;
    line.second.y += (direction[1]/dirLen) * elongation;

    elongatedLine = line;
    return elongatedLine;
}

float CrossingDetector::getAngleOfIntersectionRCS(Coord line1_P1, Coord line1_P2, Coord line2_P1, Coord line2_P2) {
    // Calculate m on the basis of two points of each line
    float m1 = (line1_P2.y - line1_P1.y) / (line1_P2.x - line1_P1.x);
    // LOG_DEBUG<<"line1_P2.y = "<<line1_P2.y<<", line1_P1.y = "<<line1_P1.y;
    // LOG_DEBUG<<"line1_P2.x = "<<line1_P2.x<<", line1_P1.x = "<<line1_P1.x;
    // LOG_DEBUG<<"m1 is "<<m1;
    float m2 = (line2_P2.y - line2_P1.y) / (line2_P2.x - line2_P1.x);
    // LOG_DEBUG<<"m2 is "<<m2;
    float angle_between_lines = atan(std::abs((m2 - m1) / (1 + m1 * m2))); // in radians
    // LOG_DEBUG<<"(m2-m1) is "<<(m2-m1);
    // LOG_DEBUG<<"(1+m1*m2)is "<<(1+m1*m2);
    // LOG_DEBUG<<"angle_between_lines is "<<angle_between_lines;

    //return the absolute value of the angle
    return angle_between_lines;
}

} //namespace bbvision


/*

Crossing detection in RCS (findTandLCrossings):
1. Determine crossing points in ICS
    Done in ICS in order to be able to use vision results to draw the lines and crossings.

2. Convert lines and crossings to RCS
    Done in order to have more accurate results when checking the crossings.

3. Do all the testing of the crossings in RCS
    a. Check crossings using distance between points on lines and crossings.
        For each line of the crossing, the point closer to the crossing (closerPoint) and the point
        further from the crossing (furtherPoint) are determined. If closerPoint is further than 
        MAX_DIST_LINE_TO_CROSSING away from the crossing, the crossing is not valid.

    b. Check crossings using angles between the two crossing lines
        The allowed angle error is determined using the point with the smallest y-coordinate - The smaller
        the coordinate, the more angle error is allowed. Crossings with an angle error larger than this 
        are not valid.

4. Determine which kind of crossing the found crossings are
    The order of points on a line is determined using the distance between furtherPoint and closerPoint/intersection. 
    If both lines of the crossing have the points in the order "point-point-intersection", 
    it is an L Crossing. If one line has the points in the order "point-intersection-point", 
    and the other in the order "point-point-intersection", it is a T-Crossing.
    
    If the closer point to the T-Crossing on the point-intersection-point axis is closer than MIN_DIST_POINT_TO_TCROSSING, 
    the crossing is sorted out.

*/

