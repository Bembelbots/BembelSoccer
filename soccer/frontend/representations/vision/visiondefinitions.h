#pragma once

#include <iosfwd>
#include <framework/serialize/serializer.h>

#define PERFECT_COLOR_BALL 220
#define PERFECT_COLOR_FIELD 100
#define PERFECT_COLOR_RECT 150
#define PERFECT_COLOR_GOAL 200
#define PERFECT_COLOR_LINE 255
#define PERFECT_COLOR_BAND 50
#define PERFECT_COLOR_PITCH_CORRECTION 40
#define PERFECT_COLOR_CROSS 75
#define PERFECT_COLOR_ROI 60

#define PERFECT_RGB_BALL_R 255
#define PERFECT_RGB_BALL_G 128
#define PERFECT_RGB_BALL_B 0
#define PERFECT_RGB_BAND_R 255
#define PERFECT_RGB_BAND_G 0
#define PERFECT_RGB_BAND_B 255
#define PERFECT_RGB_FIELD_R 0
#define PERFECT_RGB_FIELD_G 255
#define PERFECT_RGB_FIELD_B 0
#define PERFECT_RGB_GOAL_R 255
#define PERFECT_RGB_GOAL_G 255
#define PERFECT_RGB_GOAL_B 0
#define PERFECT_RGB_RECT_R 0
#define PERFECT_RGB_RECT_G 0
#define PERFECT_RGB_RECT_B 255
#define PERFECT_RGB_LINE_R 255
#define PERFECT_RGB_LINE_G 255
#define PERFECT_RGB_LINE_B 255
#define PERFECT_RGB_CROSS_R 255
#define PERFECT_RGB_CROSS_G 0
#define PERFECT_RGB_CROSS_B 255


#undef ENUM_NAME
#undef ENUM_LIST
/// enumeration for our different things to search for

enum VisionClass {
    JSVISION_NOTHING, ///< nothing,                             0
    JSVISION_FIELD, ///< field                                  1
    JSVISION_BALL, ///< ball                                    2
    JSVISION_RECT, ///< rect, mostly for debugging              3
    JSVISION_GOAL, ///< goal pole                               4
    JSVISION_LINE, ///< field line                              5
    JSVISION_LCROSS, ///< crossing lines                        6
    JSVISION_TCROSS,  //                                        7
    JSVISION_XCROSS, //                                         8
    JSVISION_PENALTY, //                                        9
    JSVISION_CIRCLE, ///< circle line                           10
    JSVISION_FACE, ///< human face                              11
    JSVISION_SCANPOINT, ///< a scanpoint from the ScanLines     12
    JSVISION_ROBOT ///< a robot on the field                    13
};

/** this should be a class, too
 * to set default constructors.
 */
class VisionResult {
public:
    /**
     * defaul constructor
     */
    VisionResult() :
        type(JSVISION_NOTHING),
        timestamp(0),
        ics_x1(0),
        ics_y1(0),
        ics_x2(0),
        ics_y2(0),
        ics_width(0),
        ics_height(0),
        ics_confidence(0.0f),
        rcs_x1(0.0f),
        rcs_y1(0.0f),
        rcs_x2(0.0f),
        rcs_y2(0.0f),
        rcs_alpha(0.0f),
        rcs_distance(0.0f),
        rcs_confidence(0.0f),
        camera(0),
        extra_int(0),//this extra int should be removed
        extra_float(0.0f) {};

    /**
     * create jsvisionresult from string.
     */
    explicit VisionResult(const std::string &vals);

    VisionClass type; ///< type of vision result
    int timestamp; ///< Timestamp for this vision result

    // ics releated stuff
    int ics_x1; ///< x coordinate of vision result in ICS
    int ics_y1; ///< y coordinate of vision result in ICS
    /**
     * x coordinate of second object.
     * this is empty for ball, but line needs a endpoint.
     * if a goal with two poles was found, this stores the second pole x position.
     */
    int ics_x2;
    /**
     * y coordinate of second object.
     * this is empty for ball, but line needs a endpoint.
     * if a goal with two poles was found, this stores the second pole y position.
     */
    int ics_y2;
    int ics_width; ///< width of found object
    int ics_height; ///< height of found object
    float ics_confidence; ///< confidence of object we've found

    // wcs releated stuff
    float rcs_x1; ///< RCS x coordinate of this object
    float rcs_y1; ///< RCS y coordinate of this object
    float rcs_x2; ///< RCS x coordinate of second object (\see ics_x2 for more details)
    float rcs_y2; ///< RCS y coordinate of second object (\see ics_y2 for more details)
    float rcs_alpha; ///< angle in RAD to object in RCS
    float rcs_distance; ///< distance in m to object in RCS
    float rcs_confidence; ///< confidence of object we've found

    // source camera created this vision result
    int camera;

    // some additional fields
    /**
    * extra integer value.
    * Here some further informations are stored. this can be
    * poleside for a goal vision result (1 left pole, 0 right pole, -1 don't know.
    * crossing type (please fill in types here becker)
    */
    int extra_int;

    float extra_float; ///< additional float value (for what?)

    /**
     * to string function.
     * used by save BLD image function
     */
    friend std::ostream &operator<<(std::ostream &, const VisionResult &);

};

SERIALIZE(std::vector<VisionResult>, {
    int elements = value.size();
    int size = sizeof(elements) + (elements * sizeof(VisionResult));
    unsigned char *tmp = new unsigned char[size];
    std::memcpy(tmp, &elements, sizeof(elements));
    int position = sizeof(elements);
    for (size_t i = 0; i < value.size(); ++i) {
        std::memcpy(tmp + position, &(value[i]), sizeof(VisionResult));
        position += sizeof(VisionResult);
    }
    SerializedT ret { key, base64_encode(tmp, size)};
    delete[] tmp;
    return ret;
});

using VisionResultVec = std::vector<VisionResult>;

// vim: set ts=4 sw=4 sts=4 expandtab:
