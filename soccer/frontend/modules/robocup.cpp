#include "robocup.h"
#include <framework/rt/kernel.h>

#include "motion/motion.h"
#include "whistle/whistle.h"
#include "behavior/behavior.h"
#include "gamecontrol/gamecontrol.h"
#include "localization/pose.h"
#include "teamcomm/teamcomm.h"
#include "vision/vision.h"
#include "worldmodel/worldmodel.h"

void RoboCup::connect(rt::Linker &link) {
    link.name = "Robocup";
}

void RoboCup::load(rt::Kernel &soccer) {
    modules.emplace_back(new Whistle());
    modules.emplace_back(new Gamecontrol());
    modules.emplace_back(new Pose());
    modules.emplace_back(new TeamComm());
    modules.emplace_back(new MotionModule());
    modules.emplace_back(new Behavior());
    modules.emplace_back(new Vision());
    modules.emplace_back(new WorldModel());

    for(auto &module : modules) {
        soccer.load(module.get());
    }
}
