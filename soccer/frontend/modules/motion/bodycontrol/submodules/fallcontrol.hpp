#pragma once

#include <bodycontrol/internals/submodule.h>
#include <framework/filter/exponential_moving_average.h>

#include <representations/bembelbots/constants.h>
#include <framework/math/angle.h>

class FallControl : public SubModule {
public:
    SubModuleReturnValue step(BodyBlackboard *bb) override {
        // tuning parameters
        static constexpr float THRESHOLD_MS{1000};    // activation threshold (milliseconds until torso is horizontal)
        static const Angle maxBodyAngle = 70.0_deg;   // if torso angle is greater than this, bot is always fallen
        static const Angle fallingRotation = 0.5_deg; // assume bot is stable, if gyro speed is below this value
        const auto &gyro{bb->sensors.imu.gyroscope};

        static EMA gyroRoll{0.f, 0.9f};
        static EMA gyroPitch{0.f, 0.9f};

        gyroRoll += gyro.x();
        gyroPitch += gyro.y();

        Angle roll_angle{Rad{fabsf(bb->bodyAngles.x())}};
        Angle pitch_angle{Rad{fabsf(bb->bodyAngles.y())}};

        bb->qns[IS_FALLEN] = false;
        bb->qns[IS_FALLING] = false;
        bb->fallenSide = FallenSide::NONE;

        switch (bb->activeMotion) {
            // robot is never fallling, while trying to standup
            case Motion::STAND_UP_FROM_FRONT: // fallthrough
            case Motion::STAND_UP_FROM_BACK:
                LOG_DEBUG_EVERY_N(100) << "STANDING UP";
                bb->qns[IS_STANDING_UP] = true;
                break;

            default:
                if (pitch_angle > maxBodyAngle || roll_angle > maxBodyAngle) {
                    float acc_x = fabsf(bb->sensors.imu.accelerometer.x());
                    bb->qns[IS_FALLEN] = true;
                    bb->fallenSide = (bb->bodyAngles(1) < 0.0f) ? FallenSide::FRONT : FallenSide::BACK;
                    bb->qns[IS_FALLING] = fabsf(std::max(gyroPitch, gyroRoll)) > fallingRotation.rad();
                } else if (gyroPitch != 0.0f || gyroRoll != 0.0f) {
                    float t1 = (M_PI_2_F - pitch_angle.rad()) / fabsf(gyroPitch);
                    float t2 = (M_PI_2_F - roll_angle.rad()) / fabsf(gyroRoll);
                    int t = static_cast<int>(std::min(t1, t2) * 1000);

                    if (t < THRESHOLD_MS)
                        bb->qns[IS_FALLING] = true;
                }
                break;
        }        

        return RUNNING;
    }
};
