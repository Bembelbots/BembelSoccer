#include <iostream>
#include <cmath>
#include <cstring>
#include "unittest.hpp"
using namespace std;


#define TEXT_NORMAL "\033[0m"
#define TEXT_BOLD_RED "\033[1;31m"
#define TEXT_BOLD_YELLOW "\033[1;33m"
#define TEXT_BOLD_GREEN "\033[1;32m"
#define TEXT_BOLD_BLUE "\033[1;34m"
#define TEXT_RED "\033[0;31m"
#define TEXT_YELLOW "\033[0;33m"
#define TEXT_GREEN "\033[0;32m"
#define TEXT_BLUE "\033[0;34m"
#define TEXT_HEADLINE TEXT_RED
#define TEXT_SEPARATOR TEXT_BOLD_BLUE
#define TEXT_DATA TEXT_YELLOW
#define TEXT_PASSED TEXT_BOLD_GREEN
#define TEXT_FAILED TEXT_BOLD_RED
#define EL TEXT_NORMAL << std::endl
#define TN EL

#define INFO std::cout << "[INFO] " << TEXT_NORMAL
#define WARN std::cout << "[WARN] " << TEXT_BOLD_YELLOW
#define FAIL std::cout << "[FAIL] " << TEXT_RED
#define HEAD std::cout << "[HEAD] " << TEXT_BOLD_BLUE
#define GOOD std::cout << "[GOOD] " << TEXT_BOLD_GREEN
#define COMMENT std::cout << TEXT_YELLOW

//#define die LOG_ERROR << "would be critical" << EL;
#define die \
{};

inline bool eq(float a, float b) {
    return abs(a - b) <= abs(a) * 1e-6;
}
inline bool eqz(float a) {
    if (a == 0.f || a == -0.f)
        return true;
    if (std::abs(a) < 1e-6)
        return true;
    if (isnormal(a))
        return false;
    return false;
}


#include <bodycontrol/bodycontrol.h>
#include <bodycontrol/bodycontrol.cpp>
#include <bodycontrol/command.cpp>
#include <bodycontrol/environment/interpolation.cpp>
#include <bodycontrol/modules.h>
#include "state_machine_mockup.hpp"
#include <bodycontrol/utils/joints.hpp>
#include <bodycontrol/utils/joint_interpolation.h>
#include <bodycontrol/command.h>
#include <bodycontrol/internals/topological_sort.cpp>
#include <bodycontrol/internals/cognition_buffer.h>
#include <bodycontrol/internals/cognition_buffer.cpp>
#include <bodycontrol/test/cognition_buffer_multithreading_test.cpp>
#include <bodycontrol/internals/json_bodycontrol.hpp>
#include <bodycontrol/utils/stiffness_control.hpp>
#include <bodycontrol/internals/motion_logger.hpp>

int main() {
    std::cout << EL << EL<< EL;

    COMMENT << "Build:" << EL;
    //COMMENT << BUILD_ID_STRING << EL;
    HEAD <<" ----- Test Begin -----" << EL;
    if (true){
            { UnitTest unitTest("UnitTest Selftest");
                TEST << "foo"; TEST << 3;
                UNIT << "fo*"; UNIT << "[0-9]";
            }
        if (false) { // Additional Selftests
            {   UnitTest unitTest("Should Fail");
                TEST << "foo";
            }
            {   UnitTest unitTest("Should Fail");
                TEST << "foo";
                UNIT << "foo"; UNIT << "bar";
            }
            {   UnitTest unitTest("Should Fail");
                TEST << "foo";
                UNIT << "bar";
            }
            {   UnitTest unitTest1("Unit Test1");
                TEST << "foo"; UNIT << "foo";
                UnitTest unitTest2("Unit Test2");
                TEST << "bar"; UNIT << "bar";
            }
        }
    }



    if (true){
        HEAD << " ----- BodyControl ----- " << EL;
        BodyControl body;
        UnitTest ut("Body Control Test", false, false);
        MODULE_ID mid = ZERO;
        float sensors[SensorDataSize];
        float actuators[ActuatorDataSize];
        memset(actuators, 0, ActuatorDataSize*sizeof(float));
        memset(sensors, 0, sizeof(sensors));
        sensors[0] = 1;
        //UNIT << "init";
        //UNIT << "sequence"; UNIT << "NONE";
        //UNIT << "ZERO.*"; UNIT << "TEST1.*"; UNIT << "TEST2.*"; UNIT << "TEST3.*";
        //UNIT_CHECKPOINT;
        //body.issueCommand(TEST1);
        //UNIT << "issue TEST1.*";
        //UNIT_CHECKPOINT;
        body.step(actuators,sensors);
        //body.issueCommand(TEST2)["lol"];
        //UNIT << "issue TEST2.*";
        //UNIT_CHECKPOINT;
        sensors[0] = 2;
        body.step(actuators,sensors);
        body.step(actuators,sensors);
        body.step(actuators,sensors);
        body.issueCommand(TEST1)["foo"]=2;
        UNIT << "issue.*TEST1.*";
        body.step(actuators,sensors);
        UNIT << "step TEST1";
        body.step(actuators,sensors);
        UNIT << "step TEST1";
        body.issueCommand(TEST2);
        body.step(actuators,sensors);
        UNIT << "issue.*TEST2.*";
        UNIT << "step TEST2";

        //for (auto x : actuators){ DBG << x << EL; }
    }
    if (true){
        HEAD << " ----- Joints ----- " << EL;
        { UnitTest ut("Joints Test");
            Joints<All> j({1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5});
            Joints<Legs> l({1,2,3,4,5,6,7,8,9,0,1});
            Joints<Arms> a({1,2,3,4,5,6,7,8,9,0,1,2});
            Joints<Head> h({1,2});
            TEST << j.size();
            UNIT << 25;
            TEST << l.size();
            UNIT << 11;
            TEST << a.size();
            UNIT << 12;
            TEST << h.size();
            UNIT << 2;
            Joints<Head> h2(h);
            TEST << h2.values[0]; UNIT << 1;
            Joints<Head> h3 = JointsDeg<Head>({45,180});
            TEST << h3.values[0] << " " << h3.values[1];
            UNIT << "0.7.* 3.14.*";
        }
        { UnitTest ut("Joints Size Add Test");
            Joints<All> j({1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5});
            Joints<Legs> l({1,2,3,4,5,6,7,8,9,0,1});
            Joints<Arms> a({1,2,3,4,5,6,7,8,9,0,1,2});
            Joints<Arms> ab({1,2,3,4,5,6,7,8,9,0,1,2});
            Joints<Head> h({1,2});
            TEST << (a+h).size(); UNIT << 14;
            TEST << (a+ab).size(); UNIT << 12;
            for (auto i : (a+ab).values) TEST << i;
            for (auto i : {2,4,6,8,10,12,14,16,18,0,2,4}) UNIT << i;
            for (auto i : (h+l).values) TEST << i;
            for (auto i : {1,2,1,2,3,4,5,6,7,8,9,0,1}) UNIT << i;
            auto hp = h.subchain<HeadPitch>();
            TEST << hp.values[0]; UNIT << 2;
            TEST << hp.size(); UNIT << 1;
        }
        { UnitTest ut("JointInterpolation Test");
            JointInterpolation<Head> inter({0,2},{2,0},4);
            auto j = inter.getJoints(0);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "0 2";
            j = inter.getJoints(1);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "0.* 1.*";
            j = inter.getJoints(2);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "1 1";
            j = inter.getJoints(3);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "1.* 0.*";
            j = inter.getJoints(4);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "2 0";
        }
        { UnitTest ut("JointInterpolationMulti Test");
            vector< Joints<Head> > jvec({{1,2}, {1,3}, {2,3}});
            JointInterpolationMulti<Head> inter(jvec, vector<float>({2,5}));
            auto j = inter.getJoints(0);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "1 2";
            j = inter.getJoints(1);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "1 2.5";
            j = inter.getJoints(2);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "1 3";
            j = inter.getJoints(4.5);
            TEST << j.values[0] << " " << j.values[1];
            UNIT << "1.5 3";
        }
    }
    if (true){
        HEAD << " ----- JsonBodyCommander ----- " << EL;
        //UnitTest ut("Body Command Test");
        BodyControl bc;
        JsonBodyCommander jbc(&bc);
        string message(
            "{\"debugv2\":{\
                \"msg_type\":\"BodyCommand\",\
                \"MODULE_ID\":\"WALK\",\
                \"key\":\"stop\"\
            }}");
        jbc.parseAndIssue(message);
        //UNIT << ".*WALK.*" << EL;
        
    }
    if (true){
        HEAD << " ----- BodyCommand ----- " << EL;
        UnitTest ut("Body Command Test");
        CommandBuilder command(NULL, NONE);
    }
    if (true){
        HEAD << " ----- Cognition Buffer ----- " << EL;
        { UnitTest ut("Cognition Buffer Test");
            CognitionBuffer buffer;
            BodyBlackboard bb;
            CognitionMessage msg;
            buffer.test(); UNIT << "r: 0 w: 0";
            bb.timestamp_ms = 10; buffer.write(bb);
            buffer.test(); UNIT << "r: 0 w: 1";
            bb.timestamp_ms = 20; buffer.write(bb);
            buffer.test(); UNIT << "r: 0 w: 2";
            msg = buffer.readClosest(20);
            buffer.test(); UNIT << "r: 1 w: 2";
            bb.timestamp_ms = 30; buffer.write(bb);
            buffer.test(); UNIT << "r: 1 w: 3";
            bb.timestamp_ms = 40; buffer.write(bb);
            buffer.test(); UNIT << "r: 1 w: 4";
            bb.timestamp_ms = 48; buffer.write(bb);
            buffer.test(); UNIT << "r: 1 w: 5";
            bb.timestamp_ms = 62; buffer.write(bb);
            buffer.test(); UNIT << "r: 1 w: 6";
            bb.timestamp_ms = 70; buffer.write(bb);
            buffer.test(); UNIT << "r: 1 w: 7";
            bb.timestamp_ms = 80; buffer.write(bb);
            buffer.test(); UNIT << "r: 1 w: 8";
            msg = buffer.readClosest(50);
            buffer.test(); UNIT << "r: 4 w: 8";
            TEST << msg.timestamp_ms; UNIT << "48";
            msg = buffer.readClosest(56);
            buffer.test(); UNIT << "r: 5 w: 8";
            TEST << msg.timestamp_ms; UNIT << "62";
            msg = buffer.readLast();
            buffer.test(); UNIT << "r: 7 w: 8";
            TEST << msg.timestamp_ms; UNIT << "80";
            bb.timestamp_ms = 90; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 9";
            bb.timestamp_ms = 100; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 0";
            bb.timestamp_ms = 110; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 1";
            bb.timestamp_ms = 120; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 2";
            bb.timestamp_ms = 130; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 3";
            bb.timestamp_ms = 140; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 4";
            bb.timestamp_ms = 150; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 5";
            bb.timestamp_ms = 160; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 6";
            bb.timestamp_ms = 170; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 6";
            bb.timestamp_ms = 180; buffer.write(bb);
            buffer.test(); UNIT << "r: 7 w: 6";
            msg = buffer.readLast();
            buffer.test(); UNIT << "r: 5 w: 6";
            TEST << msg.timestamp_ms; UNIT << "160"; // Not 180! we're still writing to w: 6
            bb.timestamp_ms = 190; buffer.write(bb);
            buffer.test(); UNIT << "r: 5 w: 7";
            msg = buffer.readLast();
            TEST << msg.timestamp_ms; UNIT << "190";
        }
        if (true){
            UnitTest ut("Cognition Buffer Multithreading Test");
            cognition_buffer_multithreading_test();
        }
    }
    if (true){
        HEAD << " ----- Topological Sorting ----- " << EL;
        { UnitTest ut("Topological Sort Test", false, false);
            vector<unsigned int> rank;
            vector<int> sequence;
            vector< tuple<int, int> > niceness;
            niceness.push_back( make_tuple(0,0));
            niceness.push_back( make_tuple(1,0));
            niceness.push_back( make_tuple(2,-2));
            niceness.push_back( make_tuple(3,1));
            niceness.push_back( make_tuple(4,0));
            vector<initializer_list<int> > dependencies(5,{});
            dependencies[2]={1};
            vector<const char *> debugnames({"MODULE 0","MODULE 1","MODULE 2","MODULE 3","MODULE 4"});
            bool ret;
            ret = topological_sort(rank, sequence, niceness, dependencies, debugnames);
            TEST << (ret?"good":"bad");     UNIT << "good";

            TEST << "sequence";     for (auto x : sequence){ TEST << x; }
            TEST << "rank";         for (auto r : rank){ TEST << r; }
            
            UNIT << "sequence";     for (auto x : {1,2,0,4,3}){ UNIT << x;}
            UNIT << "rank";         for (auto x : {3,1,2,5,4}){ UNIT << x;}
        }
        if (true)
        { UnitTest ut("Topological Sort Cyclic Dependency Test", false, false);
            vector<unsigned int> rank;
            vector<int> sequence;
            vector< tuple<int, int> > niceness;
            niceness.push_back( make_tuple(0,0));
            niceness.push_back( make_tuple(1,0));
            niceness.push_back( make_tuple(2,-2));
            niceness.push_back( make_tuple(3,1));
            niceness.push_back( make_tuple(4,0));
            vector<initializer_list<int> > dependencies(5,{});
            dependencies[2]={1};
            dependencies[1]={3};
            dependencies[3]={2};
            vector<const char *> debugnames({"MODULE 0","MODULE 1","MODULE 2","MODULE 3","MODULE 4"});
            bool ret;
            ret = topological_sort(rank, sequence, niceness, dependencies, debugnames);
            TEST << (ret?"good":"bad");     UNIT << "bad";
        }
    }
    if (true){
        { UnitTest ut("Stand Motion", false, false);
            BodyBlackboard bb;
        }
    }
    if (true){
        HEAD << " ----- State Machines ----- " << EL;
        UnitTest ut("State Machine Test", false);
        BCSM bcsm = BCSM();
        bcsm.step();
        UNIT << "initial init";
        UNIT << "initial_state initial";
        UNIT << "returned: 0";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "initial_state initial";
        UNIT << "returned: 0";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "initial_state initial";
        UNIT << "returned: 0";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "state 1 init";
        UNIT << "state 1";
        UNIT << "returned: 1";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "state 1";
        UNIT << "returned: 1";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "state 1";
        UNIT << "returned: 1";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "state 1";
        UNIT << "immediate state 1";
        UNIT << "immediate state 2";
        UNIT << "returned: 3";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "state 2 init";
        UNIT << "state 2";
        UNIT << "returned: 4";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "state 2";
        UNIT << "returned: 4";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "state 2";
        UNIT << "returned: 4";
        UNIT_CHECKPOINT bcsm.step();
        UNIT << "initial init";
        UNIT << "initial_state initial";
        UNIT << "returned: 0";


    }
    {
        BodyBlackboard bb;
        setStiffness(0.8, &bb);
    }
    {   
        HEAD << " ----- LogCollector ------ " << EL;
        UnitTest ut("LogCollector");
        //LogCollector();
        //LogCollector(motion_logger_network_singleton);
        M_LOG << "foo";
        DBG << motion_logger_cout_singelton << EL;
        DBG << motion_logger_network_singleton << EL;
        //TEST << motion_logger_cout_singleton->getContent();
    }

    HEAD << " -----Test end-----" << EL;
}
