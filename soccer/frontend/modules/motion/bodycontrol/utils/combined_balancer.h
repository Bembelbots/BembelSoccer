#pragma once

#include <vector>
#include <framework/filter/exponential_moving_average.h>
#include <fstream>

class CombinedBalancer {
 public:
    CombinedBalancer();
    ~CombinedBalancer();
  void proceed(const float &gx, const float &gy, std::vector<float> d);
  float getShoulderPitch() const { return shoulderPitch; }
  float getShoulderRollL() const { return shoulderRollL; }
  float getShoulderRollR() const { return shoulderRollR; }
  float getAnklePitch() const { return anklePitch; }
  float getAnkleRoll() const { return ankleRoll; }


 private:
  EMA shoulderPitch{0.f, 0.05f};
  EMA shoulderRollL{0.f, 0.5f};
  EMA shoulderRollR{0.f, 0.5f};

  float anklePitch = 0.0;
  float ankleRoll = 0.0;

  float duration = 10.0;
  EMA gyroPitch{0.f, 0.7f};
  EMA gyroRoll{0.f, 0.7f};
  
  /* logging for calibration: */
  // std::ofstream lf;
};
