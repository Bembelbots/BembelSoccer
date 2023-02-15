#include <string>
#include <vector>
#include <iostream>
#include <cstring>

#ifndef JRLSOCCER2_SRC_BBCOMMANDS_H_
#define JRLSOCCER2_SRC_BBCOMMANDS_H_

#define MAX_FUNCTION_ARGUMENTS_LEN 1024

#include <representations/bembelbots/types.h>

enum BBControlId {
    controlIdNothing = 0,
    controlIdSay,
    controlIdInterpolate,
    controlIdStaticMotion,
    controlIdMax
};

class BBControlDataBase {
public:

    virtual ~BBControlDataBase() {}

    virtual BBControlDataBase getData() {
        BBControlDataBase t;
        std::memcpy(t.blob, blob, sizeof(blob));
        std::memcpy(&t.tick, &tick, sizeof(tick));
        std::memcpy(&t.controlId, &controlId, sizeof(controlId));
        return t;
    }

    virtual void initFromBase(BBControlDataBase *base) {
        std::memcpy(blob, base->blob, sizeof(blob));
        std::memcpy(&tick, &base->tick, sizeof(tick));
        std::memcpy(&controlId, &base->controlId, sizeof(controlId));
    }

    virtual void serialize() {
    }

    virtual void unserialize() {
    }

    BBControlId controlId;
    char blob[MAX_FUNCTION_ARGUMENTS_LEN];
    int32_t tick;
};

///**
// * function call to execute static motion
// */
//class BBFunctionStaticMotion : public BBControlDataBase {
//public:
//    explicit BBFunctionStaticMotion(BBControlDataBase *base) {
//        initFromBase(base);
//        unserialize();
//    }
//
//    explicit BBFunctionStaticMotion(MotionID what, float arg)
//        : m_id(what), argument(arg) {
//
//        // be sure to set unique id!
//        controlId = controlIdStaticMotion;
//
//        // serialize this class!
//        serialize();
//    }
//
//    virtual ~BBFunctionStaticMotion() {
//    }
//
//    void serialize() {
//        std::memcpy(blob, &m_id, sizeof(MotionID));
//        std::memcpy(blob + sizeof(MotionID), &argument, sizeof(float));
//    }
//
//    void unserialize() {
//        std::memcpy(&m_id, blob, sizeof(MotionID));
//        std::memcpy(&argument, blob + sizeof(MotionID), sizeof(float));
//    }
//
//    MotionID m_id;
//    float argument;
//};



/**
 * function call to say something
 */
class BBFunctionSay : public BBControlDataBase {
public:
    explicit BBFunctionSay(BBControlDataBase *base) {
        initFromBase(base);
        unserialize();
    }

    explicit BBFunctionSay(std::string what)
        : text(what) {

        len = what.size();

        // be sure to set unique id!
        controlId = controlIdSay;

        // be sure to check arguments size to fit into blob!
        if (len > MAX_FUNCTION_ARGUMENTS_LEN - sizeof(len)) {
            std::cout << "warning: text to speach is too long, cropping!" << std::endl;
            len = MAX_FUNCTION_ARGUMENTS_LEN - sizeof(len);
        }

        // be sure to serialize this class!
        serialize();
    }

    virtual ~BBFunctionSay() {
    }

    void serialize() {
        std::memcpy(blob, &len, sizeof(len));
        std::memcpy(blob + sizeof(len), text.c_str(), len);
    }

    void unserialize() {
        std::memcpy(&len, blob, sizeof(len));
        text = std::string(blob + sizeof(len), blob + sizeof(len) + len);
    }

    std::string text;
    size_t len;
};

/**
 * function call to interpolate a chain to a position in a given time
 */
class BBFunctionInterpolate : public BBControlDataBase {
public:
    explicit BBFunctionInterpolate(BBControlDataBase *base) {
        initFromBase(base);
        unserialize();
    }

    explicit BBFunctionInterpolate(std::vector<std::string> names,
                                   std::vector<float> positions, float t)
        : jointNames(names),
          nameSize(0),
          positionSize(0),
          jointPositions(positions),
          time(t) {

        nameSize = jointNames.size();
        positionSize = jointPositions.size();

        // be sure to set unique id!
        controlId = controlIdInterpolate;

        // be sure to serialize this class!
        serialize();
    }

    virtual ~BBFunctionInterpolate() {
    }

    void serialize() {
        int32_t p = 0;
        std::memcpy(blob + p, &nameSize, sizeof(nameSize));
        p += sizeof(nameSize);
        for (size_t i = 0; i < nameSize; ++i) {
            std::string s(jointNames[i]);
            size_t len = s.size();
            std::memcpy(blob + p, &len, sizeof(len));
            p += sizeof(len);
            std::memcpy(blob + p, s.c_str(), len);
            p += len;
        }
        std::memcpy(blob + p, &positionSize, sizeof(positionSize));
        p += sizeof(positionSize);
        for (size_t i = 0; i < positionSize; ++i) {
            std::memcpy(blob + p, &jointPositions[i], sizeof(float));
            p += sizeof(float);
        }
        std::memcpy(blob + p, &time, sizeof(time));
    }

    void unserialize() {
        int32_t p = 0;
        std::memcpy(&nameSize, blob + p, sizeof(nameSize));
        p += sizeof(nameSize);
        jointNames.clear();
        for (size_t i = 0; i < nameSize; ++i) {
            size_t len;
            std::memcpy(&len, blob + p, sizeof(len));
            p += sizeof(len);
            jointNames.push_back(std::string(blob + p, blob + p + len));
            p += len;
        }
        std::memcpy(&positionSize, blob + p, sizeof(positionSize));
        p += sizeof(positionSize);
        for (size_t i = 0; i < positionSize; ++i) {
            float t;
            std::memcpy(&t, blob + p, sizeof(float));
            p += sizeof(float);
            jointPositions.push_back(t);
        }
        std::memcpy(&time, blob + p, sizeof(time));
    }

    std::vector<std::string> jointNames;
    size_t nameSize, positionSize;
    std::vector<float> jointPositions;
    float time;
};

#endif //JRLSOCCER2_SRC_BBCOMMANDS_H_

// vim: set ts=4 sw=4 sts=4 expandtab:
