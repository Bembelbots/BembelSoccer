#pragma once

#include <bodycontrol/internals/submodule.h>

#include "button.h"
#include "lola_names_generated.h"


class ButtonHandler : public SubModule {
public:
    ButtonHandler() 
            : chestButton(bbapi::TouchNames::Chest_Button, 500)
            , leftBumper({bbapi::TouchNames::RFoot_Bumper_Left, bbapi::TouchNames::RFoot_Bumper_Right}, 500) 
            , rightBumper({bbapi::TouchNames::LFoot_Bumper_Left, bbapi::TouchNames::LFoot_Bumper_Right}, 500)
            , frontHeadTouch(bbapi::TouchNames::Head_Front, 500) 
            , middleHeadTouch(bbapi::TouchNames::Head_Middle, 500) 
            , rearHeadTouch(bbapi::TouchNames::Head_Rear, 500) {
    }

    SubModuleReturnValue step(BodyBlackboard * bb) override {
        chestButton.update(bb->sensors, bb->timestamp_ms);
        bb->qns[CHEST_BUTTON_PRESSED]       = chestButton.pressed;
        bb->qns[CHEST_BUTTON_DOUBLE_CLICK]  = chestButton.doubleClick;
        bb->qns[CHEST_BUTTON_TRIPPLE_CLICK] = chestButton.trippleClick;
        bb->qns[CHEST_BUTTON_DOWN]          = chestButton.isDown;
        bb->qns[CHEST_BUTTON_UP]            = chestButton.isUp;

        leftBumper.update(bb->sensors, bb->timestamp_ms);
        bb->qns[LEFT_BUMPER_PRESSED]       = leftBumper.pressed;
        bb->qns[LEFT_BUMPER_DOUBLE_CLICK]  = leftBumper.doubleClick;
        bb->qns[LEFT_BUMPER_TRIPPLE_CLICK] = leftBumper.trippleClick;
        bb->qns[LEFT_BUMPER_DOWN]          = leftBumper.isDown;
        bb->qns[LEFT_BUMPER_UP]            = leftBumper.isUp;

        rightBumper.update(bb->sensors, bb->timestamp_ms);
        bb->qns[RIGHT_BUMPER_PRESSED]       = rightBumper.pressed;
        bb->qns[RIGHT_BUMPER_DOUBLE_CLICK]  = rightBumper.doubleClick;
        bb->qns[RIGHT_BUMPER_TRIPPLE_CLICK] = rightBumper.trippleClick;
        bb->qns[RIGHT_BUMPER_DOWN]          = rightBumper.isDown;
        bb->qns[RIGHT_BUMPER_UP]            = rightBumper.isUp;

        frontHeadTouch.update(bb->sensors, bb->timestamp_ms);
        bb->qns[FRONT_HEAD_TOUCH_PRESSED]       = frontHeadTouch.pressed;
        bb->qns[FRONT_HEAD_TOUCH_DOUBLE_CLICK]  = frontHeadTouch.doubleClick;
        bb->qns[FRONT_HEAD_TOUCH_TRIPPLE_CLICK] = frontHeadTouch.trippleClick;
        bb->qns[FRONT_HEAD_TOUCH_DOWN]          = frontHeadTouch.isDown;
        bb->qns[FRONT_HEAD_TOUCH_UP]            = frontHeadTouch.isUp;

        middleHeadTouch.update(bb->sensors, bb->timestamp_ms);
        bb->qns[MIDDLE_HEAD_TOUCH_PRESSED]       = middleHeadTouch.pressed;
        bb->qns[MIDDLE_HEAD_TOUCH_DOUBLE_CLICK]  = middleHeadTouch.doubleClick;
        bb->qns[MIDDLE_HEAD_TOUCH_TRIPPLE_CLICK] = middleHeadTouch.trippleClick;
        bb->qns[MIDDLE_HEAD_TOUCH_DOWN]          = middleHeadTouch.isDown;
        bb->qns[MIDDLE_HEAD_TOUCH_UP]            = middleHeadTouch.isUp;

        rearHeadTouch.update(bb->sensors, bb->timestamp_ms);
        bb->qns[REAR_HEAD_TOUCH_PRESSED]       = rearHeadTouch.pressed;
        bb->qns[REAR_HEAD_TOUCH_DOUBLE_CLICK]  = rearHeadTouch.doubleClick;
        bb->qns[REAR_HEAD_TOUCH_TRIPPLE_CLICK] = rearHeadTouch.trippleClick;
        bb->qns[REAR_HEAD_TOUCH_DOWN]          = rearHeadTouch.isDown;
        bb->qns[REAR_HEAD_TOUCH_UP]            = rearHeadTouch.isUp;

        return RUNNING;
    }

private:

    Button chestButton;
    Button leftBumper;
    Button rightBumper;
    Button frontHeadTouch;
    Button middleHeadTouch;
    Button rearHeadTouch;
};
