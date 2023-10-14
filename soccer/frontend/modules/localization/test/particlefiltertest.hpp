#include <core/util/coords.h>
#include <worldmodel/pose/particlefilter.h>
#include <worldmodel/definitions.h>
#include "logdataprocessor.hpp"

using namespace std;
#define TEXT_NORMAL "\033[0m"
#define TEXT_HEADLINE "\033[0;31m"

struct Distances{
    float mean_dist;
    float max_dist;
};

class ParticleFilterTest {
public:
    /*
    set
    - robot_id
    - has_kockoff
    - role
    - num_particles
    - odo_variance
    - odo_start
    - start_position
    */
    FieldSize fieldSize = FieldSize::JRL;
    ParticleFilter::Settings conf;

    ParticleFilterTest():
    conf(new PlayingField(fieldSize)){}

    Distances runParticleFilter(LogDataset dataset, const string &outFile, bool use_events = true, bool verbose = true) {
        //conf.numParticles = 1;
        conf.startMcsPosition = dataset.data.at(2).odo.at(0);
        ParticleFilter loca(conf);
        int cognition_step = 0; 
        vector<Robot> positionOutputsAndTargets; //for erroranalyzis
        if (verbose)
            cout << "run particle filter on field with length :" <<conf.pf->_lengthInsideBounds<<endl;
        
        if (use_events){
            loca.emit_event(ParticleFilter::EV_STATE_INITIAL);
        }
        else{
            loca.emit_event(ParticleFilter::EV_STATE_PLAYING);
            loca.setPosition(dataset.data.at(2).wcs.GTpos);
        }
        DirectedCoord odometry(0.0f, 0.0f, 0.0_rad);
        for (LogData &data : dataset.data) {
            vector<VisionResult> vrs;
            cognition_step++;
            dataset.output.push_back(data);

            //Events:
            if (use_events){
                for (auto event : data.event ) {
                    if (verbose)
                        cout<< "Emit event "<< (int)(event)<<endl;;
                    loca.emit_event(event);
                }
            }

            //Odometrie:
            if (!data.odo.empty()) {
                odometry = DirectedCoord(data.odo.at(0));
            }
            /*else{
                if (verbose)
                    cout <<"no odo found at step: "<< cognition_step-1<<endl;
                //calculate odometry         
                if ((cognition_step >2)){
                    DirectedCoord lastPosition(dataset.data.at(cognition_step-2).wcs.pos);
                    DirectedCoord currPosition(dataset.data.at(cognition_step-1).wcs.pos);
                    DirectedCoord diff = currPosition.toRCS(lastPosition);
                    diff.angle.rad = currPosition.angle.rad - lastPosition.angle.rad;
                    //cout<< "odo"<< diff<<endl;
                    odometry = odometry.walk(diff);
                    dataset.output.at(dataset.output.size() -1 ).odo.push_back(odometry);
                }
            }*/

            //poseEstimates:
            pair<vector<DirectedCoord>,int> poseEstimates={{},1};
            for (auto r: data.poseEstimates){
                dataset.output.at(dataset.output.size() -1 ).poseEstimates.push_back(r);
                poseEstimates.first.push_back(r.pos);
            }

            for (auto &vr :  dataset.output.at(dataset.output.size() -1).visionResults){
                vr.extra_float = vr.extra_float - data.wcs.GTpos.angle.rad();
            }

            //update Loca
            loca.update(data.visionResults, odometry, poseEstimates);
            
            //getPos:
            Robot outpos(loca.get_position(),data.wcs.GTpos);
            positionOutputsAndTargets.push_back(outpos);
            dataset.output.at(dataset.output.size() -1).wcs = outpos;

            //get partikels
            vector<DirectedCoord> particles = loca.getHypothesesVector();
            for (auto h: particles) {
                Robot r;
                r.pos = h;
                r.confidence = 1;
                dataset.output.at(dataset.output.size() -1).hypos.push_back(r);
            }
        }

        if (!outFile.empty())
            dataset.logtoFile(outFile);

        vector<float> distances = evaluatePositionsWithGroundtruth(positionOutputsAndTargets);
        if (distances.size() > 0){
            return {mean(distances), max(distances)};
        }
        return {-1,-1};//mean_dist, max_dist
    }

    // to show initial/penalized/manPlacement handler
    
    int initFilter(std::string filename) {
        ofstream out(filename);
        DirectedCoord odometry(0.0f, 0.0f, 0.0_rad);

        conf.has_kickoff = true;
        ParticleFilter loca(conf);
        for (int type = 0; type <=5; type++) { //type =0-5 : normal for id type //6:: penalizes
            if (type <= 4) {
                loca.conf.robot_id = type;
                loca.emit_event(ParticleFilter::EV_STATE_INITIAL);
            }
            if (type == 5) {
                loca.unpenalizedHandler();  
            }
            
            out << "(" << type << ")" << " enter new cognition step: " << type << endl;

            Robot outpos(loca.get_position());

            // write hypos
            vector<DirectedCoord>  hypos = loca.getHypothesesVector();
            for (auto h: hypos) {
                Robot r;
                r.pos = h;
                r.confidence = 1;
                out << "(" << type<< ") Hypo: "  << r << endl; 
            }
            // write  estimated position
            out << "(" << type << ") WCS: " << outpos << endl; 
        }
        out.close();
        return 0;
    }

private:
    static float mean(const vector<float> &v) {
        float sum =  accumulate(v.begin(), v.end(), 0.0);
        float mean = sum / v.size();
        return mean;
    }

    static float max(const vector<float> &v) {
        //int idx_max =  std::max_element(v.begin(), v.end()) - v.begin();
        //cout<<"max at index: "<< idx_max<<endl;
        return *std::max_element(v.begin(), v.end());
    }

    static vector<float> evaluatePositionsWithGroundtruth(const vector<Robot> &positions) {
        vector<float> dist;
        vector<float> angledist;
        float stdev;
        for (size_t i=0; i < positions.size(); i++) {
            Coord pos = Coord(positions.at(i).pos.coord.x, positions.at(i).pos.coord.y);
            Coord GTpos = Coord(positions.at(i).GTpos.coord.x,
                                positions.at(i).GTpos.coord.y);
            if (GTpos.x !=0 or GTpos.y !=0){
                //distance
                dist.push_back(pos.dist(GTpos));
                //angle
                float alpha = positions.at(i).pos.angle.rad();
                float GTalpha = positions.at(i).GTpos.angle.rad();
                if ((alpha>0 and GTalpha<0) or ((alpha < 0 and GTalpha > 0))) {
                    angledist.push_back(min(alpha+GTalpha, 2*M_PI_F-(alpha+GTalpha)));
                } else {
                    angledist.push_back(fabsf(fabsf(alpha)- fabsf(GTalpha)));
                }
            }
        }
        return dist;
    }
};
