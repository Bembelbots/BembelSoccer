#include <core/util/coords.h>
#include <worldmodel/pose/particlefilter.h>
#include <worldmodel/definitions.h>
#include "logdataprocessor.hpp"
#include "particlefiltertest.hpp"

//using namespace std;

#define TEXT_NORMAL "\033[0m"
#define TEXT_HEADLINE "\033[0;31m"


int main(int argc, const char *argv[]) {

    cout<< "hi, this is the loca testbox" << TEXT_NORMAL<<endl;
    ParticleFilterTest test;

    string data_fn;
    if (argc > 1 && argv[1] != NULL){
        if (argc > 2 && argv[2] != NULL) {
            data_fn =  string(argv[1]);
            
            int id = stoi(argv[2]);
            test.conf.robot_id = id;
            //parse logfile
            LogDataset log(data_fn);
            if (log.data.size() >0){
                Distances result =  test.runParticleFilter(log, "ParticleFilterOutput.log");
                cout <<  TEXT_HEADLINE << "EVALUTAION OF LOCA FILTER, mean dist: "<<
                    result.mean_dist<< ", max_dist : "<< result.max_dist<<
                    TEXT_NORMAL << TEXT_HEADLINE << "end of test"<< endl <<
                    "---------------------------------------------------------"
                    << TEXT_NORMAL<< endl;

                return 0;
            }
            else{
                cout <<  TEXT_HEADLINE << "Couldn't read cognitionsteps from logfile!! :/"<<
                    TEXT_NORMAL<< endl;
                return 1;
            }
        }
        else{
            cout<< TEXT_HEADLINE <<  "no robot id provided"<<TEXT_NORMAL<<endl;
        }
        
    } 
    else {
        cout<< TEXT_HEADLINE <<  "no logfile provided, test initializations"<<TEXT_NORMAL<<endl;
        return test.initFilter("ParticleFilterOutput.log");
    }
}

/* Code to save matched Landmarks instead of visionResults

dataset.output.at(dataset.output.size() -1).visionResults.clear();//LogData ld;
std::vector<Feature> matchedLandmarks= loca.matchedLandmarks;
for (auto landmark: matchedLandmarks){
    if (landmark.type = JSVISION_LINE){
        LandmarkLine pfLine = conf.pf->_lines[landmark.id];
        VisionResult vrsLine;
        vrsLine.type= JSVISION_LINE;
        auto rcs1= DirectedCoord(pfLine.start_x,pfLine.start_y,0).toRCS(outpos.GTpos);
        vrsLine.rcs_x1= rcs1.coord.x;
        vrsLine.rcs_y1= rcs1.coord.y;
        auto rcs2= DirectedCoord(pfLine.end_x,pfLine.end_y,0).toRCS(outpos.GTpos);
        vrsLine.rcs_x2= rcs2.coord.x;
        vrsLine.rcs_y2= rcs2.coord.y;
        cout <<"line:"<< landmark.id
             <<",pos:"<<Coord(pfLine.start_x,pfLine.start_y)
             <<"robot at:"<< outpos.pos
             <<",rcspos:"<< rcs1.coord<<endl;

        dataset.output.at(dataset.output.size() -1).visionResults.push_back(vrsLine);
    }
}*/ 

/*  //CODE  to tranfrom a dataset
    int step =0;
    DirectedCoord odometry(0.0f, 0.0f, 0.0f);
    for (LogData &data : log.data) {
        for (auto &vr :  data.visionResults){
            if ((vr.type == JSVISION_LCROSS)|| 
                (vr.type == JSVISION_TCROSS)||
                (vr.type == JSVISION_XCROSS)) {
                if (vr.type == JSVISION_LCROSS &&vr.extra_int == 3)
                    vr.type = JSVISION_TCROSS;

                if (vr.type == JSVISION_LCROSS && vr.extra_int == 4)
                    vr.type = JSVISION_XCROSS;
        
                vr.extra_float = vr.extra_float - data.wcs.GTpos.angle.rad;
            }
        }
        if (data.odo.empty()){
            if (step >1){
                DirectedCoord lastPosition(log.data.at(step-1).wcs.pos);
                DirectedCoord currPosition(log.data.at(step).wcs.pos);
                DirectedCoord diff = currPosition.toRCS(lastPosition);
                diff.angle.rad = currPosition.angle.rad - lastPosition.angle.rad;
                //cout<< "odo"<< diff<<endl;
                odometry = odometry.walk(diff);
                data.odo.push_back(odometry);
            }
        }
        log.output.push_back(data);
        step++;
    }

    log.logtoFile("newSimulationSeq.log"); 

*/  