#pragma once

#include <framework/filter/exponential_moving_average.h>

class AnkleBalancer {
 public:
    AnkleBalancer();
    ~AnkleBalancer();
  void proceed(const float &gx, const float &gy);
  float getPitch() const { return pitch; }
  float getRoll() const { return roll; }

 private:
  float forward_gyro_gain=0.05;
  float backward_gyro_gain=0.1;
  float roll_gyro_gain=0.06;
  float pitch = 0;
  float roll = 0;
  EMA gyroPitch{0.f, 0.7f};
  EMA gyroRoll{0.f, 0.7f};
};
