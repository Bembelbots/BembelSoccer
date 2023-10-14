#include "hypothesesgenerator.h"
#include <representations/bembelbots/constants.h>
#include <algorithm>

using namespace std;

HypothesesGenerator::HypothesesGenerator(const PlayingField *field )
   : playingfield(field){}


pair<vector<DirectedCoord>,int> HypothesesGenerator::createHypotheses(const vector<VisionResult> &vrs){
    vector< vector<DirectedCoord> > hypotheses;
    for (auto vr: vrs) { //vision results from that exact positions could be calculated
        if (vr.type == JSVISION_LCROSS or  vr.type == JSVISION_TCROSS or vr.type == JSVISION_CIRCLE){
            if (vr.type == JSVISION_CIRCLE){
                circlePreprocessing(vr, vrs);
            }
            if (vr.rcs_distance < 1.5f){
                vector<DirectedCoord> tmp_estimates = createHypothesesOfALandmark(vr);
                hypotheses.push_back(tmp_estimates);
                if (vr.type == JSVISION_CIRCLE){ //if circle weight twice
                    hypotheses.push_back(tmp_estimates);
                }
            }
        }
    }
    return uniteHypothesesOfMultipleLandmarks(hypotheses);
}


//TODO this calculates the orientation of the centercircle , should be done in another (previous) module..
void HypothesesGenerator:: circlePreprocessing(VisionResult &circle, const vector<VisionResult> &vrs){
    Coord circle_pos(circle.rcs_x1, circle.rcs_y1);
    VisionResult closestLine;
    float mindist = 100.f;
    for (auto vr:vrs){
        if (vr.type == JSVISION_LINE){
            float tmp_dist = circle_pos.dist(circle_pos.closestPointOnLine(Coord(vr.rcs_x1,vr.rcs_y1),Coord(vr.rcs_x2,vr.rcs_y2), true));
            if (tmp_dist < mindist){
                closestLine = vr;
                mindist = tmp_dist;
            }
        }
    }
    Coord line = Coord(closestLine.rcs_x1, closestLine.rcs_y1)-Coord(closestLine.rcs_x2, closestLine.rcs_y2);
    circle.extra_float = line.direction().rad();
}


vector<DirectedCoord> HypothesesGenerator::createHypothesesOfALandmark(const VisionResult &vr){
    float orientation = vr.extra_float;
    vector<DirectedCoord> estimates;

    vector<LandmarkCross> map_crosses;
    if (vr.type == JSVISION_TCROSS){
        map_crosses = playingfield->getTCrosses();  
    }
    else if (vr.type == JSVISION_LCROSS){
        map_crosses = playingfield->getLCrosses();        
    }
    else if (vr.type == JSVISION_CIRCLE){
        LandmarkCross cross1;
        cross1.wcs_x = 0.0f;
        cross1.wcs_y = 0.0f;
        cross1.wcs_alpha = M_PI/2.0f;
        map_crosses.push_back(cross1);

        LandmarkCross cross2;
        cross2.wcs_x = 0.0f;
        cross2.wcs_y = 0.0f;
        cross2.wcs_alpha = -M_PI/2.0f;
        map_crosses.push_back(cross2);
    }
    
    for (uint i=0;i < map_crosses.size(); i++){
        DirectedCoord map_cross(map_crosses.at(i).wcs_x, map_crosses.at(i).wcs_y, Rad{map_crosses.at(i).wcs_alpha});
        map_cross.angle -= Rad{orientation - vr.rcs_alpha}; //wcs direction from robot
        DirectedCoord estimate = map_cross.walk(DirectedCoord(-vr.rcs_distance, 0.0f, 0.0_rad));
        estimate.angle = estimate.angle - Angle(Rad{vr.rcs_alpha});
        estimates.push_back(estimate);
    }
    return estimates;
}

pair<vector<DirectedCoord>,int> HypothesesGenerator::uniteHypothesesOfMultipleLandmarks(
                                     const std::vector<std::vector<DirectedCoord>> &hypos){
    //CHECK IF HYPOS OF different Landmarks are unitebla else return empty vector:
    vector<DirectedCoord> unitedHypos;
    int highest_matching = 0;
    if (hypos.size()>0 ){
        vector<vector <int>> number_of_matchings;
        int i_l1 = 0;
        for (uint i_landm1 = 0; i_landm1 < hypos.size(); i_landm1++){
            i_l1++;
            int i_h1 = 0;
            number_of_matchings.push_back(vector<int>());
            for (auto hypo_l1: hypos.at(i_landm1)){
                i_h1++;
                int i_l2=0;
                int matching_landmark = 1;
                for (uint i_landm2 = i_landm1+1; i_landm2 < hypos.size(); i_landm2++){
                    i_l2++;
                    int i_h2 = 0;
                    for (auto hypo_l2: hypos.at(i_landm2)){
                        i_h2++;
                        if ((hypo_l1.coord.dist(hypo_l2.coord) < max_dist) and
                                (fabs(hypo_l1.angle.dist(hypo_l2.angle).rad()) < max_angledist)) {
                            //found a landmark that leads to matching hypos
                            matching_landmark++;
                            continue; // go on with next landmark
                        }
                    }
                }
                number_of_matchings.at(i_landm1).push_back(matching_landmark);
            }
        }

        //find highest matching numbers;
        for (auto list_matchings : number_of_matchings){
            int tmp_highest_matching = *std::max_element(list_matchings.begin(), list_matchings.end());
            if (tmp_highest_matching > highest_matching)
                highest_matching= tmp_highest_matching;
        }
        //unitedHypos
        if ((highest_matching == 0) and (hypos.size()>1) ){
            return {unitedHypos, 0};
        }
        for (uint i_landm = 0; i_landm < hypos.size(); i_landm++){
            for (uint i_hypo = 0; i_hypo < hypos.at(i_landm).size(); i_hypo++){
                if (number_of_matchings.at(i_landm).at(i_hypo) == highest_matching)
                    unitedHypos.push_back(hypos.at(i_landm).at(i_hypo));
            }
        }     
    }    
    return {unitedHypos,highest_matching};
}
