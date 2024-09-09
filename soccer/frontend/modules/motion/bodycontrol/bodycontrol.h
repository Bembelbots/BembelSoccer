#pragma once
#include <representations/blackboards/settings.h>
#include <vector>
#include <queue>
#include <memory>
#include <cstddef>
#include <cstdarg>
#include <bitset>

#include <bodycontrol/blackboards/bodyblackboard.h>
#include <bodycontrol/modules.h>
#include <representations/motion/motion.h>
#include <bodycontrol/internals/submodule.h>

//#include <core/messages/body_state.h>

class BodyCommand;
class SettingsBlackboard;

namespace bbapi {
    class BembelIpcSensorMessageT;
    class BembelIpcActuatorMessageT;
}

class BodyControl {
public:
    BodyControl();      //!< Load submodules and build the sequence.
    ~BodyControl();     //!< Yes, it destroys the submodules too.

public:
    /**************************/
    /* The Complete Interface */
    /**************************/
    void connect(rt::Linker &);
    void setup(SubModule::Setup);

    /*! 
     * 1. sensor data is copied to the blackboard
     * 2. enqueued commands are processed by the submodules
     * 3. submodules run
     * 4. cognition data is written to a buffer
     * 5. actuator data is written
     */
    BodyState step(rt::Command<BodyCommand, rt::Handle> &, bbapi::BembelIpcActuatorMessageT *actuatorData, const bbapi::BembelIpcSensorMessageT *sensorData); //!< Do a step

    void activateModule(MODULE_ID);

    template<class T>
    T *getModule(MODULE_ID id) {
        return reinterpret_cast<T *>(submodule[id]);
    }

private:
    /* The Blackboard */
    BodyBlackboard bodyblackboard;

    const SettingsBlackboard *settings = nullptr;

    /* submodule management variables */
    typedef MODULE_ID rank_T; 
    std::vector<SubModule*> submodule;
    std::vector<const char*> debugname;
    std::vector<MODULE_ID> sequence;
    std::vector<rank_T> rank;
    std::bitset<NO_OF_BODY_MODULES> active;
    std::vector<Motion> motion_id;
    bool motion_stable;
    MODULE_ID activeMotion;
    MODULE_ID nextMotion;

    /* internal functions */
    void pullSensorData();
    void pushActuatorData();
    void replaceMotionWithNextMotion();
    void executeSubmodules();
    void doMotion(DoMotion);

    Motion idToMotion(MODULE_ID);
    MODULE_ID motionToId(Motion);

    inline bool isMotion(MODULE_ID id) { return motion_id[id] != Motion::NONE; }

    BodyState bbToState() const;
};
