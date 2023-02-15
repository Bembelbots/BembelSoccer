//
// Created by felix on 21.04.18.
//
#pragma once

#include <framework/util/enum/serializable_enum.h>


SERIALIZABLE_ENUM(WalkAction,
                  (OMNIDIRECTIONAL),
                  (BACKWARDS),
                  (FORWARDS),
                  (TURN),
                  (TURNLEFT),
                  (TURNRIGHT),
                  (TIPPLE),
                  (ALIGN),
                  (SIDE_LEFT),
                  (SIDE_RIGHT),
                  (TURN_AROUND),
                  (RAW),
                  (INSTEP_KICK_LEFT),
                  (INSTEP_KICK_RIGHT)
);
