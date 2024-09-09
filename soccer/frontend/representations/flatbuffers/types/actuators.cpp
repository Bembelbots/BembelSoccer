#include <array>
#include <flatbuffers/array.h>
#include <iomanip>
#include <ostream>

#include "actuators.h"
#include "actuators_generated.h"
#include "sensors.h"

namespace flatbuffers {

using bbipc::copyArray;

bbapi::Actuators Pack(const bbipc::Actuators &act) {
    // joints
    bbapi::JointActuators joints(act.joints.position, act.joints.stiffness);

    // LED chest
    bbapi::LEDSingle chest(act.led.chest.r, act.led.chest.g, act.led.chest.b);

    // LED ears
    bbapi::LEDEars ears(act.led.ears.left, act.led.ears.right);

    // LED eyes
    bbapi::LEDEye leye(act.led.eyes.left.r, act.led.eyes.left.b, act.led.eyes.left.b);
    bbapi::LEDEye reye(act.led.eyes.right.r, act.led.eyes.right.b, act.led.eyes.right.b);
    bbapi::LEDEyes eyes(leye, reye);

    // LED feet
    bbapi::LEDSingle lfoot(act.led.feet.left.r, act.led.feet.left.b, act.led.feet.left.b);
    bbapi::LEDSingle rfoot(act.led.feet.right.r, act.led.feet.right.b, act.led.feet.right.b);
    bbapi::LEDFeet feet(lfoot, rfoot);

    // join all LEDs
    bbapi::LED led(chest, ears, eyes, feet, act.led.skull);

    // sonar
    bbapi::SonarActuators sonar(act.sonar.left, act.sonar.right);

    return bbapi::Actuators(joints, led, sonar);
}

bbipc::Actuators UnPack(const bbapi::Actuators &act) {
    bbipc::Actuators ipc;

    // joints
    copyArray(ipc.joints.position, act.joints().position());
    copyArray(ipc.joints.stiffness, act.joints().stiffness());

    // LED chest
    const auto &cled{act.led().chest()};
    ipc.led.chest = {cled.r(), cled.g(), cled.b()};

    // LED ears
    copyArray(ipc.led.ears.left, act.led().ears().left());
    copyArray(ipc.led.ears.right, act.led().ears().right());

    // LED left eye
    copyArray(ipc.led.eyes.left.r, act.led().eyes().left().r());
    copyArray(ipc.led.eyes.left.g, act.led().eyes().left().g());
    copyArray(ipc.led.eyes.left.b, act.led().eyes().left().b());

    // LED right eye
    copyArray(ipc.led.eyes.right.r, act.led().eyes().right().r());
    copyArray(ipc.led.eyes.right.g, act.led().eyes().right().g());
    copyArray(ipc.led.eyes.right.b, act.led().eyes().right().b());

    // LED feet
    const auto &lfoot{act.led().feet().left()};
    ipc.led.feet.left = {lfoot.r(), lfoot.g(), lfoot.b()};
    const auto &rfoot{act.led().feet().right()};
    ipc.led.feet.right = {rfoot.r(), rfoot.g(), rfoot.b()};

    // LED skull
    copyArray(ipc.led.skull, act.led().skull());

    // sonar
    ipc.sonar.left = act.sonar().left();
    ipc.sonar.right = act.sonar().left();

    return ipc;
}
} // namespace flatbuffers

std::ostream &operator<<(std::ostream &s, const bbipc::Actuators &a) {
    s << std::fixed << std::setprecision(2);
    s << "=== ActuatorsMsg:" << std::endl;

    s << "   Joints:" << std::endl;
    s << "      Position:  ";
    for (const float f : a.joints.position)
        s << f << "  ";
    s << std::endl;

    s << "      Stiffness: ";
    for (const float f : a.joints.stiffness)
        s << f << "  ";
    s << std::endl;

    s << "   LEDs:" << std::endl;
    s << "      Chest:     ";
    s << "(" << a.led.chest.r << ", " << a.led.chest.g << ", " << a.led.chest.b << ")" << std::endl;

    s << "      LEar:      ";
    for (const float f : a.led.ears.left)
        s << f << "  ";
    s << std::endl;

    s << "      REar:      ";
    for (const float f : a.led.ears.right)
        s << f << "  ";
    s << std::endl;

    s << "      LEye:      ";
    const auto leye{a.led.eyes.left};
    for (size_t i{0}; i<leye.NUM_LEDS; ++i)
        s << "(" << leye.r[i] << ", " << leye.g[i] << ", " << leye.b[i] << ")  ";
    s << std::endl;

    s << "      REye:      ";
    const auto reye{a.led.eyes.right};
    for (size_t i{0}; i<reye.NUM_LEDS; ++i)
        s << "(" << reye.r[i] << ", " << reye.g[i] << ", " << reye.b[i] << ")  ";
    s << std::endl;

    s << "      Feet:      ";
    s << "(" << a.led.feet.left.r << ", " << a.led.feet.left.g << ", " << a.led.feet.left.b << ")";
    s << "  ";
    s << "(" << a.led.feet.right.r << ", " << a.led.feet.right.g << ", " << a.led.feet.right.b << ")";
    s << std::endl;
    
    s << "   Sonar:        ";
    s << a.sonar.left << "\t" << a.sonar.right << std::endl;
    s << std::endl;

    return s;
}
