#pragma once

// pi (double)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// pi (float)
#define M_PI_F 3.14159265358979323846f

// pi/2 (double)
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

// pi/2 (float)
#ifndef M_PI_2_F
#define M_PI_2_F 1.57079632679489661923f
#endif

#ifndef M_PI_4_F
#define M_PI_4_F (M_PI_2_F / 2.f)
#endif

// factor to be multiplied with a radiant to convert it to degree
#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0f / M_PI_F)
#endif
// and vice-versa
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (M_PI_F / 180.0f)
#endif
