/**
 * @author Jonathan Cyriax Brast
 *
 * This file is the one place to register new BodyControl SubModules.
 * To register a new motion search for the TODO comments.
 *
 * If you need to know how this file works, look into bodycontrol.cpp
 */

// clang-format off

#ifdef I_AM_BODY_CONTROL
    
    /////////////////////////////
    // TODO:                   //
    // Include your file here: //
    /////////////////////////////
    
    #include <bodycontrol/submodules/init_actuators.hpp>
    #include <bodycontrol/submodules/reset_body_questions.hpp>
    #include <bodycontrol/submodules/campose.hpp>
    
    #include <bodycontrol/submodules/walk/walk_wrapper.hpp>
    #include <bodycontrol/submodules/kick/kick_v3.hpp>
    #include <bodycontrol/submodules/sensor_dump.hpp>
    //#include <bodycontrol/submodules/hold_position.hpp>
    //#include <bodycontrol/submodules/led_disco.hpp>
    #include <bodycontrol/submodules/stand/sit.hpp>
    #include <bodycontrol/submodules/stand/stand.hpp>
    #include <bodycontrol/submodules/do_nothing.hpp>
    //#include <bodycontrol/submodules/test_module.hpp>
    //#include <bodycontrol/submodules/timestamp.hpp>
    //#include <bodycontrol/submodules/live_test.hpp>
    #include <bodycontrol/submodules/head_motion/head_motion.hpp>
    #include <bodycontrol/submodules/buttons/buttonhandler.hpp>
    #include <bodycontrol/submodules/led_control/led_control.hpp>
    #include <bodycontrol/submodules/fallcontrol.hpp>
    #include <bodycontrol/submodules/odometry.hpp>
    #include <bodycontrol/submodules/stiffness/stiffness.hpp>

    #include <bodycontrol/submodules/filters/ahrs.hpp>

    #include <bodycontrol/submodules/sensors/ground_contact.hpp>

    #include <bodycontrol/submodules/stabilization/stabilization.hpp>
    #include <bodycontrol/submodules/motion_design/motion_design_engine.h>

    #include <bodycontrol/submodules/special_motions/bbmf_motion.hpp>

    #include <bodycontrol/submodules/obstacle_detector/obstacle_detector.hpp>
    #include <bodycontrol/submodules/energy_saver.hpp>
#endif


#ifndef _MODULE_ID
    #include <representations/motion/motion.h>
    #include <framework/util/enum/serializable_enum.h>


    //////////////////////////////////////////////////
    // TODO:                                        //
    // Get your new submodule a new MOTION_ID here: //
    //////////////////////////////////////////////////
    SERIALIZABLE_ENUM(MODULE_ID,
        (NONE) (0),           // must be first

        // initialization
        (INIT_ACTUATORS),

        // Calculations
        (KINEMATICS),
        (CENTER_OF_MASS),
        (BUTTON),
        (LED_CTRL),
        //CAMERA_TRANSFORMATION,
        (AHRS),
        (ODOMETRY),
        (GROUND_CONTACT),

        // Reflexes
        (FALLCONTROL),
        (STABILIZATION),

        (STIFFNESS),

        // Main motions
        (BC_WALK),
        
        (SIT),
        (INTERPOLATE_TO_STAND),
        //PENALIZED_STANCE,
        (GOALIE_THROW_LEFT),
        (GOALIE_THROW_RIGHT),
        (KICK_LEFT),
        (KICK_RIGHT),
        
        (STAND_UP_FROM_FRONT),
        (STAND_UP_FROM_BACK),
        (RAISE_ARM),

        // Additional modules
        (HEAD_MOTION),
        (RESET_BODY_QNS),

        // Preprocessing
        //HOLD_POSITION,

        // Postprocessing

        // Motion Design
        (MOTION_DESIGN),

        // Obstacle Detector
        (OBSTACLE_DETECTOR),

        // Proof of concept stuff
        (LED_DISCO),
        // LIVE_TEST,
        //DUMP_HEAD_YAW,
        // (DUMP_LSHOULDER_ROLL),
        // (DUMP_LSHOULDER_PITCH),

        // Test and debug modules
        // ZERO,
        // TEST1,
        // TEST2,
        // TEST3,
        
        (CAMPOSE),
        (ENERGY_SAVER),

        (NO_OF_BODY_MODULES) // must be last
    );

#define _MODULE_ID
#endif


#ifdef REGISTER_SUBMODULES_NOW
    //////////////////////////////////////////////////////
    // TODO:                                            //
    // Register your modules or motions like that:      //
    //   MODULE(MODULE_ID, ModuleClass, ...)            //
    //      or                                          //
    //   MOTION(MODULE_ID, ModuleClass, ...)            //
    //     ... parametes for the initializer            //
    // optional options:                                //
    //   NICENESS(MODULE_ID, niceness )                 //
    //       niceness > 0 will be executed later        //
    //       niceness < 0 will be executed earlier      //
    //   DEPENDENCY(MODULE_ID, ...)                     //
    //     ... other module ids                         // 
    //       assures execution after other modules      //
    //////////////////////////////////////////////////////
    // === The Difference between MODULE and MOTION === //
    // MODULE and MOTION are similar statements and     //
    // work pretty much the same. The difference is:    //
    //                                                  //
    //      --- MODULE ---                              //
    // Every submodule registered with MODULE runs      //
    // every cycle. They should return one the          //
    // return values described in <summodule.h>.        //
    //                                                  //
    //      --- MOTION ---                              //
    // Only one submodule registered with MOTION may    //
    // run in one cycle. A motion should return:        //
    //      + MOTION_UNSTABLE                           //
    //      + MOTION_STABLE                             //
    // Returning MOTION_UNSTABLE will protect the       //
    // motion from deactivation. It should only return  //
    // MOTION_STABLE in a position where the body is    //
    // stable. If another motion is on hold the         //
    // active motion will be deactivated and the        //
    // motion on hold will be activated.                //
    //////////////////////////////////////////////////////

    MODULE( INIT_ACTUATORS, InitActuators)
        NICENESS( INIT_ACTUATORS, -300)
    // This module should always run first
    // Add a dependency to this module, if your module sets
    // a body question
    MODULE( RESET_BODY_QNS, ResetBodyQuestions)
        NICENESS( RESET_BODY_QNS, -300)

    // Filters -200
    MODULE( AHRS, AHRSFilter)
        NICENESS( AHRS, -200)

    MODULE(STABILIZATION, Stabilization)
        NICENESS(STABILIZATION, -70)
        DEPENDENCY(STABILIZATION, STIFFNESS)

    MODULE(OBSTACLE_DETECTOR, ObstacleDetector)
        NICENESS(OBSTACLE_DETECTOR, -100)
        DEPENDENCY(OBSTACLE_DETECTOR, RESET_BODY_QNS)

    MOTION(BC_WALK, Motion::WALK, WalkWrapper)
        NICENESS(BC_WALK, -50)
        DEPENDENCY(BC_WALK, AHRS)
        DEPENDENCY(BC_WALK, RESET_BODY_QNS)
        DEPENDENCY(BC_WALK, STIFFNESS)

    MODULE(ODOMETRY, DoNothing)
        NICENESS(ODOMETRY, -10)
    
    MODULE(GROUND_CONTACT, GroundContact)

    MODULE(BUTTON, ButtonHandler)
        NICENESS( BUTTON, -100)
        DEPENDENCY( BUTTON, RESET_BODY_QNS)
    MODULE(LED_CTRL, LedControl)
        NICENESS( LED_CTRL, -100)
        DEPENDENCY(LED_CTRL, FALLCONTROL)

    MODULE( KINEMATICS, DoNothing)
    MODULE( CAMPOSE, CalcCamPose)
    MODULE( CENTER_OF_MASS, DoNothing)
    MODULE( FALLCONTROL, FallControl)
        DEPENDENCY( FALLCONTROL, RESET_BODY_QNS)
        NICENESS( FALLCONTROL, -20)

    MODULE( STIFFNESS, Stiffness)
        DEPENDENCY( STIFFNESS, FALLCONTROL)
    
    // MODULE( CAMERA_TRANSFORMATION, DoNothing)
    //    NICENESS( CAMERA_TRANSFORMATION, -30)

    MODULE(HEAD_MOTION, HeadMotion)

    // Dummy Motion so stuff compiles
    MOTION (KICK_LEFT, Motion::KICK_LEFT, BBMFMotion, "kick_l.bbmf", true, true, true)
        DEPENDENCY(KICK_LEFT, STIFFNESS)
    //MOTION(KICK_RIGHT, kick_v3_r)
    MOTION(KICK_RIGHT, Motion::KICK_RIGHT, BBMFMotion, "kick_r.bbmf", true, true, true)
        DEPENDENCY(KICK_RIGHT, STIFFNESS)
    MOTION (GOALIE_THROW_LEFT, Motion::GOALIE_THROW_LEFT, BBMFMotion, "GoalieThrowLeft.bbmf")
        DEPENDENCY(GOALIE_THROW_LEFT, STIFFNESS)
    MOTION (GOALIE_THROW_RIGHT, Motion::GOALIE_THROW_RIGHT, BBMFMotion, "GoalieThrowRight.bbmf")
        DEPENDENCY(GOALIE_THROW_RIGHT, STIFFNESS)

    MOTION(MOTION_DESIGN, Motion::DESIGNER, MotionDesignEngine)
        DEPENDENCY(MOTION_DESIGN, STIFFNESS)
        DEPENDENCY(MOTION_DESIGN, HEAD_MOTION)

    MOTION (SIT, Motion::SIT, Sit)
        DEPENDENCY( SIT, RESET_BODY_QNS)
        DEPENDENCY( SIT, STIFFNESS)

    MOTION (INTERPOLATE_TO_STAND, Motion::INTERPOLATE_TO_STAND, InterpolateToStand)
        DEPENDENCY( INTERPOLATE_TO_STAND, RESET_BODY_QNS)
        DEPENDENCY( INTERPOLATE_TO_STAND, STIFFNESS)

    MOTION(STAND_UP_FROM_FRONT, Motion::STAND_UP_FROM_BACK, BBMFMotion, "stand_up_from_back.bbmf")
        DEPENDENCY( STAND_UP_FROM_FRONT, STIFFNESS)
    MOTION(STAND_UP_FROM_BACK, Motion::STAND_UP_FROM_FRONT, BBMFMotion, "stand_up_from_front.bbmf")
        DEPENDENCY( STAND_UP_FROM_BACK, STIFFNESS)

    MOTION(RAISE_ARM, Motion::RAISE_ARM, BBMFMotion, "raiseArm.bbmf")
        DEPENDENCY(RAISE_ARM, RESET_BODY_QNS)
        DEPENDENCY(RAISE_ARM, STIFFNESS)
    //MOTION (PENALIZED_STANCE, DoNothing)
    //    NICENESS (PENALIZED_STANCE, -5)
    
    // MODULE(DUMP_LSHOULDER_PITCH, SensorDumper<lShoulderPitchPositionSensor>, "shoulder pitch")
    // MODULE(DUMP_LSHOULDER_ROLL, SensorDumper<lShoulderRollPositionSensor>, "shoulder roll")

    // MOTION (DUMP_HEAD_YAW, SensorDumper<headYawPositionSensor>)

    //MOTION (HOLD_POSITION, HoldPosition)
    //NICENESS (HOLD_POSITION, -100)

    MODULE(LED_DISCO, DoNothing)

    // MOTION (LIVE_TEST, LiveTest)
    
    MODULE(ENERGY_SAVER, EnergySaver)
        NICENESS(ENERGY_SAVER, 9999)
#endif

// clang-format on
// vim: set ts=4 sw=4 sts=4 expandtab:
