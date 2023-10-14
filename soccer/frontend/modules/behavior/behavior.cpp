#include "behavior.h"
#include <framework/rt/kernel.h>
//#include "simple/simplebehavior.h"
#include "soccer/behaviorcontrol.h"

void Behavior::connect(rt::Linker &link) {
    link.name = "Behavior";
}

void Behavior::load(rt::Kernel &soccer) {
    behavior = std::shared_ptr<rt::Module>(new BEHAVE_PRIVATE::BehaviorControl());
    soccer.load(behavior.get());
}
