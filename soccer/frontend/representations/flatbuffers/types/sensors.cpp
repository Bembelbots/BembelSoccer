#include "sensors.h"
#include "sensors_generated.h"
#include <Eigen/src/Core/Matrix.h>

namespace flatbuffers {
bbapi::Sensors Pack(const bbipc::Sensors &s) {
    // battery
    bbapi::Battery battery(s.battery.charge, s.battery.current, s.battery.status, s.battery.temperature);

    // joints
    bbapi::JointSensors joints(
            s.joints.position, s.joints.stiffness, s.joints.temperature, s.joints.current, s.joints.status);

    // FSR
    bbapi::FSR fsr(s.fsr.left.arr, s.fsr.right.arr);

    // IMU
    std::array<float, 3> accel = {s.imu.accelerometer[0], s.imu.accelerometer[1], s.imu.accelerometer[2]};
    std::array<float, 3> gyro = {s.imu.gyroscope[0], s.imu.gyroscope[1], s.imu.gyroscope[2]};
    std::array<float, 2> angles = {s.imu.angles[0], s.imu.angles[1]};
    bbapi::IMU imu(accel, gyro, angles);

    // sonar
    bbapi::SonarSensors sonar(s.sonar.left, s.sonar.right);

    // touch
    bbapi::TouchChest chest(s.touch.chest.button);
    bbapi::TouchHead head(s.touch.head.front, s.touch.head.middle, s.touch.head.rear);
    bbapi::TouchHand lhand(s.touch.hands.left.back, s.touch.hands.left.left, s.touch.hands.left.right);
    bbapi::TouchHand rhand(s.touch.hands.right.back, s.touch.hands.right.right, s.touch.hands.right.right);
    bbapi::TouchHands hands(lhand, rhand);
    bbapi::TouchFoot lfoot(s.touch.feet.left.bumper_left, s.touch.feet.left.bumper_right);
    bbapi::TouchFoot rfoot(s.touch.feet.right.bumper_left, s.touch.feet.right.bumper_right);
    bbapi::TouchFeet feet(lfoot, rfoot);
    bbapi::Touch touch(chest, head, hands, feet);

    return bbapi::Sensors(battery, joints, fsr, imu, sonar, touch);
}

bbipc::Sensors UnPack(const bbapi::Sensors &s) {
    using bbipc::copyArray;
    bbipc::Sensors ipc;

    // battery
    ipc.battery.charge = s.battery().charge();
    ipc.battery.current = s.battery().current();
    ipc.battery.status = s.battery().status();
    ipc.battery.temperature = s.battery().temperature();

    // joints
    copyArray(ipc.joints.position, s.joints().position());
    copyArray(ipc.joints.stiffness, s.joints().stiffness());
    copyArray(ipc.joints.status, s.joints().status());
    copyArray(ipc.joints.temperature, s.joints().temperature());
    copyArray(ipc.joints.current, s.joints().current());

    // fsr
    copyArray(ipc.fsr.left.arr, s.fsr().leftFoot());
    copyArray(ipc.fsr.right.arr, s.fsr().rightFoot());

    // imu
    const auto &a = *s.imu().accelerometer();
    const auto &g = *s.imu().gyroscope();
    const auto &ang = *s.imu().angles();
    ipc.imu.accelerometer << a[0], a[1], a[2];
    ipc.imu.gyroscope << g[0], g[1], g[2];
    ipc.imu.angles << g[0], g[1];

    // sonar
    ipc.sonar = {s.sonar().left(), s.sonar().right()};

    // touch
    ipc.touch.chest.button = s.touch().chest().button();
    const auto &head = s.touch().head();
    ipc.touch.head = {head.front(), head.middle(), head.rear()};
    const auto &feet = s.touch().feet();
    ipc.touch.feet = {{feet.left().bumperLeft(), feet.left().bumperRight()},
            {feet.right().bumperLeft(), feet.right().bumperRight()}};
    const auto &lhand = s.touch().hands().left();
    const auto &rhand = s.touch().hands().right();
    ipc.touch.hands = {{lhand.back(), lhand.left(), lhand.right()}, {rhand.back(), rhand.left(), rhand.right()}};

    return ipc;
}
} // namespace flatbuffers
