#pragma once

// WARNING: THIS HEADER SHOULD GENERALLY NOT BE USED!!!
// the following functions are only for simulator use and will mess things up if used outside of simulation
//  implementation in clock.cpp

#include <representations/bembelbots/types.h>

// set fake time for standalone behavior or simulator
void setGlobalTimeFromSimulation(TimestampMs);

// switch clock to simulator time
// do not use outside of main(), there is no way to go back!
// (STANDALONE_BEHAVIOR always defaults to simulator clock)
void useSimulatorClock();
