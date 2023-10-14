#pragma once

#include <vector>
#include <initializer_list>

#include <libbembelbots/bembelbots.h>

#include <framework/util/assert.h>
#include <framework/joints/joints.hpp>

class Button {
    
public:

    bool pressed = false;
    bool isDown = false;
    bool isUp = false;
    bool trippleClick = false;
    bool doubleClick = false;
    
    Button(const std::initializer_list<LBBSensorIds> buttonSensors, int timeBetweenClicks)
            : buttonIds(buttonSensors)
            , maxTimeBetweenClicks(timeBetweenClicks) {
        jsassert(!buttonIds.empty());
    }
    
    Button(const LBBSensorIds buttonSensors, int timeBetweenClicks) :
            Button({buttonSensors}, timeBetweenClicks) {}

    void update(const joints::Sensors &sensors, int timems) {
        isDown = isPressedDown(sensors);
        isUp   = !isDown;

        // clear previous state
        this->pressed = false;
        this->trippleClick = false;
        this->doubleClick = false;

        bool pressed = false;

        if (isUp) {
            pressed = false;
            wasUp   = true;
            wasDown = false;
        } else {
            pressed = wasUp;
            wasUp   = false;
            wasDown = true;
        }

        if (pressed) {
            clickCnt++;
            lastClickTime = timems;

            if (clickCnt == 3) {
                clickCnt = 0;
                trippleClick = true;
            } 
        } else if (clickCnt > 0) {
            jsassert(clickCnt < 3);

            if (timems - lastClickTime > maxTimeBetweenClicks) {
                this->pressed     = (clickCnt == 1);
                this->doubleClick = (clickCnt == 2);

                clickCnt = 0; 
            }
        }

    }

private:

    std::vector<LBBSensorIds> buttonIds;

    int maxTimeBetweenClicks = 500;

    bool wasUp   = false;
    bool wasDown = false;

    int clickCnt = 0;
    int lastClickTime = 0;

    bool isPressedDown(const joints::Sensors &sensors) {
        bool sensorActive = false;
        for (const LBBSensorIds &buttonId : buttonIds) {
            sensorActive |= (sensors[buttonId] > 0);
        }
        return sensorActive;
    }

};

// vim: set ts=4 sw=4 sts=4 expandtab:
