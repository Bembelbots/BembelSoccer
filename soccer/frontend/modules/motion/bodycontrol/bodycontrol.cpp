#include "bodycontrol.h"
#include "ipc_actuator_message_generated.h"
#include "ipc_sensor_message_generated.h"
#include <bodycontrol/internals/topological_sort.hpp>
#include <bodycontrol/utils/actuatorcheck.h>

#include <framework/util/assert.h>
#include <framework/util/clock.h>
#include <framework/util/clock_simulator.h>
#include <representations/blackboards/settings.h>

#include <iostream>
#include <mutex>
#include <cstdarg>
#include <algorithm>
#include <tuple>
#include <bitset>
#include <stack>
#include <ctime>
//#include <unistd.h>
#include <cstring>

// Includes all the submodules
#define I_AM_BODY_CONTROL
    #include "modules.h"
    #ifdef BODYCONTROL_STATE_MACHINE
        #error("State machine not undefined. \
You have to undefine the state machine after using it. \
See bodycontrol/utils/state_machine.h for more infos.")
    #endif
#undef I_AM_BODY_CONTROL

#define hcf {LOG_ERROR << "Halt and catch fire!";exit(37);}

using namespace std;

/* timed_mutex mutexed_queue_mutex; */

BodyControl::BodyControl():
    bodyblackboard(),
    submodule(NO_OF_BODY_MODULES, NULL),
    debugname(NO_OF_BODY_MODULES, NULL),
    sequence(NO_OF_BODY_MODULES, NONE),
    rank(NO_OF_BODY_MODULES, NONE),
    active(),
    motion_stable(false),
    activeMotion(NONE),
    nextMotion(NONE)
{
     LOG_DEBUG << "init Bodycontrol";

    // check special stances are valid
    jsassert(!STANDUP_STANCE.isInvalid());
    jsassert(!SIT_STANCE.isInvalid());


    /*===========================*/
    /* tmp vector initialization */
    /*===========================*/

    vector<SubModule*> init_submodule(NO_OF_BODY_MODULES,NULL);
    vector<const char*> init_debugname(NO_OF_BODY_MODULES,NULL);
    vector< vector<MODULE_ID> > init_dependencies(NO_OF_BODY_MODULES, {NONE});
    vector< tuple<MODULE_ID, int> > init_niceness(NO_OF_BODY_MODULES);
    vector<MODULE_ID> init_sequence;
    vector<unsigned int> init_rank;
    vector<Motion> init_motion_id{NO_OF_BODY_MODULES, Motion::NONE};

    for (int i = 0; i < NO_OF_BODY_MODULES; i++){
        init_niceness[i] = make_tuple((MODULE_ID)i, 0); 
    }

    init_submodule[0] = NULL;
    init_debugname[0] = "NONE";
    init_dependencies[0] = {};

    bool halt_and_catch_fire = false; //!< set if some sanity check failed

    /*======================*/
    /* Registration process */
    /*======================*/

    // clang-format off
    #define MODULE(MODULE_ID,ModuleClass, ...) \
        init_submodule[MODULE_ID] = dynamic_cast<SubModule*>( new ModuleClass(__VA_ARGS__) );\
        init_debugname[MODULE_ID] =  "Module " #MODULE_ID " " #ModuleClass "(" #__VA_ARGS__")"; \
        init_motion_id[MODULE_ID] = Motion::NONE;
    #define MOTION(MODULE_ID, MOTION_ID, ModuleClass, ...) \
        init_submodule[MODULE_ID] = dynamic_cast<SubModule*>( new ModuleClass(__VA_ARGS__) );\
        init_debugname[MODULE_ID] =  "Motion " #MODULE_ID " " #ModuleClass "(" #__VA_ARGS__")";\
        static_assert(MOTION_ID != Motion::NONE, "Motions can't be have the motion id NONE!"); \
        init_motion_id[MODULE_ID] = MOTION_ID;
    #define DEPENDENCY(MODULE_ID, ...)\
        init_dependencies[MODULE_ID] = {NONE,__VA_ARGS__};
    #define NICENESS(MODULE_ID, niceness)\
        init_niceness[MODULE_ID] = make_tuple(MODULE_ID, niceness); 
    #define REGISTER_SUBMODULES_NOW
    #include "modules.h"
    #undef REGISTER_SUBMODULES_NOW
    #undef MODULE
    #undef DEPENDENCY
    #undef NICENESS
    // clang-format on

    // Debug and sanity check
    for (int i = 1;i < NO_OF_BODY_MODULES; i++){
        if (init_submodule[i] == NULL){
            LOG_ERROR << "Submodule #" << i << " is not registered.";
            halt_and_catch_fire = true;
        }
    }
    if (halt_and_catch_fire) hcf;


    /*===================================*/
    /* Topoligical sorting of submodule */
    /*===================================*/

    if ( not topological_sort(init_rank, init_sequence, init_niceness, init_dependencies, init_debugname)) {
        LOG_ERROR << "Topological sort didn't work.";
        hcf;
    }

    /*===============================================================*/
    /* Building and checking BodyControl members from init_* vectors */
    /*===============================================================*/


    // Build BodyControl members
    sequence = init_sequence;
    submodule = init_submodule;
    debugname = init_debugname;
    motion_id = init_motion_id;
    for(int i = 0; i < NO_OF_BODY_MODULES; i++){
        rank[i] = (rank_T) (init_rank[i]-1);
        //DBG << rank[i] << " " << debugname[i];
    }

    // Debug info
    LOG_INFO << "BodyControl submodule sequence:";
    for(int i = 1 ; i < NO_OF_BODY_MODULES; i++) {
        LOG_INFO << " -> " << debugname[sequence[i]];
    }

    // Sanity checks
    for (int i = 1; i < NO_OF_BODY_MODULES; i++){
        if (rank[i] <= 0){
            LOG_ERROR << "Rank unknown for module #" << i;
            halt_and_catch_fire = true;
        }
        if (submodule[i] == NULL){
            LOG_ERROR << "No instance for module #" << i;
            halt_and_catch_fire = true;
        }
        if (debugname[i] == NULL){
            LOG_ERROR << "No name for module #" << i;
            halt_and_catch_fire = true;
        }
        if (sequence[i] == NONE){
            LOG_ERROR << "Sequence is empty at rank " << i;
            halt_and_catch_fire = true;
        }
    }
    if (halt_and_catch_fire) hcf;

    /* activate all (non-motion) MODULEs  */
    active[0] = false;
    for (size_t i = 1; i < NO_OF_BODY_MODULES; i++) {
        active[i] = not isMotion(static_cast<MODULE_ID>(i));
    }
}

BodyControl::~BodyControl(){
    for (auto & module : submodule){
        delete module;
    }
}

void BodyControl::connect(rt::Linker &meta) {
    for (auto module : submodule) {
        if (module == nullptr) {
            continue;
        }
        module->connect(meta);
    }
}

void BodyControl::setup(SubModule::Setup s) {
    /* reset all MODULEs */
    s.cmds->connect<DoMotion, &BodyControl::doMotion>(this);
    settings = s.settings;
    for (auto module : submodule) {
        if (module != nullptr) {
            module->setup(s);
            module->reset();
        }
    }
}

BodyState BodyControl::step(rt::Command<BodyCommand, rt::Handle> &commands, bbapi::BembelIpcActuatorMessageT *actuatorData, const bbapi::BembelIpcSensorMessageT *sensorData) {
    if(!actuatorData || !sensorData)
        return bbToState();

    bodyblackboard.actuators = &(actuatorData->actuators);

    bodyblackboard.sensors = sensorData->sensors;

    actuatorData->timestamp = bodyblackboard.timestamp_ms = getTimestampMs();
    bodyblackboard.lola_timestamp = sensorData->lolaTimestamp;
    const auto &simTick{bodyblackboard.sensors.battery.temperature};
    if (settings->simulator && (simTick > 0)) {
        bodyblackboard.lola_timestamp = bodyblackboard.timestamp_ms =
                simTick * CONST::lola_cycle_ms;
        setGlobalTimeFromSimulation(bodyblackboard.timestamp_ms);
    }
    bodyblackboard.connected = sensorData->connected;

    commands.update();
    for (int id = 1; id < NO_OF_BODY_MODULES; id++) {
        if (submodule[id]->activateMe) {
            activateModule(static_cast<MODULE_ID>(id));
        }
    }
    replaceMotionWithNextMotion();
    executeSubmodules();

    return bbToState();
}

void BodyControl::executeSubmodules(){
    for (size_t id_counter=1; id_counter<NO_OF_BODY_MODULES; ++id_counter){
        MODULE_ID id = sequence[id_counter];

        if (active[id]){
            /* Execution of submodule */
            SubModuleReturnValue return_value = MOTION_UNSTABLE;
            return_value = submodule[id]->step(&bodyblackboard);
            
             if(!checkAndCorrectActuators(
                         bodyblackboard.actuators, 
                         bodyblackboard.sensors)) { 
                 LOG_ERROR_EVERY_N(100) << "Module (" << enumToStr(id) << "): Actuators out of range."; 
             } 

            /* react to return value commands */
            switch (return_value) {                 
                case MOTION_UNSTABLE:
                    jsassert(isMotion(id)) << "Submodule " << id << " returned MOTION_UNSTABLE, but is not a motion.";
                    motion_stable = false;
                    break;

                case MOTION_STABLE:
                    jsassert(isMotion(id)) << "Submodule " << id << " returned MOTION_STABLE, but is not a motion.";
                    motion_stable = true;
                    break;

                case RUNNING:
                    jsassert(not isMotion(id)) << "Submodule " << id << " returned RUNNING, but is a motion.";
                    //pass
                    break;

                case END_SEQUENCE:
                    return;

                case HALT_AND_CATCH_FIRE:
                    hcf;
                    break;

                case DEACTIVATE_ME:
                    active[id] = false;
                    break;
            }
        }
    }

    if (motion_stable) replaceMotionWithNextMotion();

    bodyblackboard.activeMotion = idToMotion(activeMotion);
}

void BodyControl::doMotion(DoMotion m) {
    activateModule(motionToId(m.id));
}

void BodyControl::activateModule(MODULE_ID id) {
    if (not isMotion(id)) {
        if (not active[id]){
            active[id] = true; // activate Motion
            submodule[id]->activateMe = false;
            submodule[id]->reset();
        }
    } else {
        if (not (id == activeMotion or id == nextMotion) ) {
            nextMotion = id; // put motion on hold
        }
    }
}

void BodyControl::replaceMotionWithNextMotion(){
    if (not active[activeMotion]) activeMotion = NONE;
    if (nextMotion == NONE) return;
    if (active[activeMotion]){
        if (not motion_stable) return;
        active[activeMotion] = false;
    }
    active[nextMotion] = true;
    submodule[nextMotion]->activateMe = false;
    submodule[nextMotion]->reset();
    activeMotion = nextMotion;
    nextMotion = NONE;
}

Motion BodyControl::idToMotion(MODULE_ID id) {
    return motion_id[id];
}

MODULE_ID BodyControl::motionToId(Motion m) {
    jsassert(m != Motion::NONE);
    for (size_t i = 0; i < NO_OF_BODY_MODULES; i++) {
        if (motion_id[i] == m) {
            return static_cast<MODULE_ID>(i);
        }
    }
    JS_UNREACHABLE() << "No Motion associated with Motion ID '" << m << "'!";
}

BodyState BodyControl::bbToState() const {
    // TODO write directly to output
    const BodyBlackboard &bb = bodyblackboard;
    BodyState st;

    st.timestamp_ms = bb.timestamp_ms;
    st.tick = bb.tick;
    st.lola_timestamp = bb.lola_timestamp;

    st.sensors = bb.sensors;

    st.activeMotion = bb.activeMotion;

    st.bodyAngles = bb.bodyAngles;

    st.lastHeadYaw = bb.headYawLastPos;
    st.lastHeadPitch = bb.headPitchLastPos;

    st.fallenSide = bb.fallenSide;

    st.bCamPose = bb.bCamPose;
    st.tCamPose = bb.tCamPose;

    st.odometry = bb.odometry;

    st.com = bb.com;

    st.qns = bb.qns;

    return st;
}
