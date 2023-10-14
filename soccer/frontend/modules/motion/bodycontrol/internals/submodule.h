#pragma once
#include <cstdarg>
#include <bodycontrol/blackboards/bodyblackboard.h>
#include <representations/motion/body_commands.h>
#include <representations/motion/body_state.h>
#include <framework/rt/module.h>
#include <framework/rt/message_utils.h>

class ModuleMeta;
class SettingsBlackboard;

enum SubModuleReturnValue {
/*  Motions should return one of the following two values:
 *  MOTION_UNSTABLE
 *      "The body is in an unstable position"
 *      The default return value. This assures the motions is kept running.
 *
 *  MOTION_STABLE
 *      "The body is in a stable position"
 *      If this is returned and another motion has been requested the currently
 *      running motion is deactivated and the other motion takes it's place.
 *      If not other motion is requested the motion is kept runnning.
 */
    MOTION_UNSTABLE,        //!< "Body unstable - my job is not done - keep me running."
    MOTION_STABLE,          //!< "Body stable - someone else could take over anytime now."


/*  Modules should return one of the following values
 *  RUNNING
 *      "Eveything is ok. Nothing to see here."
 *  END_SEQUENCE
 *      "Something happened - Everyone step back - I got this."
 *      This tells the bodycontol to skip all subsequent calculations.
 *      It allows a module to take control of the bot.
 *      e.g. a fall reflex unstiffs everything.
 *  DEACTIVATE_ME
 *      "You keep telling me to do stuff. Stop it!"
 *      Deactivates the module.
 *      May be useful for modules that need to run only once / only in the beginning.
 *      e.g. a submodule that reads a configfile and writes it to the blackboard.
 *      e.g. some kind of calibration module.
 */
    RUNNING,                //!< "Everything is OK - SNAFU"
    END_SEQUENCE,           //!< "Nobody touches anything anymore!"
    DEACTIVATE_ME,          //!< "Kill me please."

/* This is the emergency killswitch. It kills the framework... */
    HALT_AND_CATCH_FIRE    //!< "Kill everything. YES! EVERYTHING!"
    // I'll leave this in here. What could possibly go wrong?
};

class SubModule {
public:
    struct Setup {
        const SettingsBlackboard *settings;
        rt::Dispatch<BodyCommand, rt::Handle> *cmds;
    };

    bool activateMe = false;

    virtual void connect(rt::Linker &) {}
    virtual void setup(Setup) {}
    virtual SubModuleReturnValue step(BodyBlackboard * bb) = 0;
    virtual void reset() {}
    virtual ~SubModule() {}
};
