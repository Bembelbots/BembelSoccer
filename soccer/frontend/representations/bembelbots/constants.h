#pragma once

#include <framework/math/constants.h>

#define TOP_CAMERA 0
#define BOTTOM_CAMERA 1

#ifndef MS_PER_SEC
#define MS_PER_SEC 1000
#endif

static constexpr unsigned char NUM_PLAYERS = 20;

// framework-wide constants
namespace CONSTANTS {

// initial head position yaw/pitch
static constexpr float initial_yaw = 0.0f;

static constexpr float initial_pitch = 0.475f;

static constexpr int max_ball_age = 5000;

static constexpr int max_robot_age = 2000;

static constexpr float robot_radius = 0.2;

static constexpr float ball_size = 0.1f;
static constexpr float robot_size = 0.3f;

// duration of a LoLa cycle for converting simulator ticks in ms
static constexpr int lola_cycle_ms = 12;

} // end CONSTANTS-namespace

namespace CONST = CONSTANTS;

// vim: set ts=4 sw=4 sts=4 expandtab:
