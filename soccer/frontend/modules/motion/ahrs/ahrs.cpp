#include <cmath>
#include <chrono>

#include "ahrs.h"
#include "MadgwickAHRS.h"
#include "representations/flatbuffers/types/sensors.h"

using namespace Madgwick;
using namespace std::chrono;
using c = std::chrono::steady_clock;

// set f to zero if < +/- eps
static constexpr float eps{0.015};
static inline void clampEps(float &f) {
    f = (abs(f) < eps) ? 0 : f;
}


AHRSProvider::AHRSProvider():
    euler(0, 0, 0) {
}


AHRSProvider::~AHRSProvider() {}


Angle AHRSProvider::getPitch() {
    return Rad{euler(0)};
}


Angle AHRSProvider::getYaw() {
    return Rad{euler(1)};
}


Angle AHRSProvider::getRoll() {
    return Rad{euler(2)};
}

Eigen::Vector3f AHRSProvider::getEuler() {
    return euler;
}

Eigen::Vector3f AHRSProvider::getAcceleration() {
    return acc;
}

void AHRSProvider::update(const bbipc::IMU &imu) {
    auto a{imu.accelerometer};
    auto g{imu.gyroscope};

    // set g to 0 if < eps to eliminate Z-axis drift
    clampEps(g.x());
    clampEps(g.y());
    clampEps(g.z());

    MadgwickAHRSupdateIMU(g.x(), g.y(), g.z(), a.x(), a.y(), a.z());

    // repeated multiplications, use variables to avoid recalculation
    float q0q0 = q0 * q0;
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q0q3 = q0 * q3;
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;

    // convert quaternion to rotation matrix
    Eigen::Matrix3f R;
    R <<  2*q0q0 - 1 + 2*q1q1,  2*(q1q2 + q0q3),     2*(q1q3 - q0q2),
          2*(q1q2 - q0q3),      2*q0q0 -1 + 2*q2q2,  2*(q2q3 + q0q1),
          2*(q1q3 - q0q2),      2*(q2q3 - q0q1),     2*q0q0 - 1 - 2*q3q3;

    // convert to euler angles
    euler << atan2f(q0q1 + q2q3, 0.5f - q1q1 - q2q2),
             asinf(-2.0f * (q1q3 - q0q2)),
             atan2f(q1q2 + q0q3, 0.5f - q2q2 - q3q3);

    a = R*a;        // transform acceleration vector to inertial frame
    a(2) -= 9.8f;    // substract gravity
    acc = a;
}


// vim: set ts=4 sw=4 sts=4 expandtab:
