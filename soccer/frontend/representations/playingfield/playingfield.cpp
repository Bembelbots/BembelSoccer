/* jsplayingfield.cpp
 * @author Christian Becker
 * Version : $Id$
 */

#include "playingfield.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include <boost/property_tree/json_parser.hpp>

#include "../bembelbots/constants.h"
#include <framework/math/old.h>
#include <framework/util/assert.h>
#include <framework/logger/logger.h>

//using namespace std;

/*
 *   _lengthInsideBounds
 *   _penaltyLength
 *      -x           x
 * |------------------------|
 * |           |            | y
 * |--|        |         |--|
 * |  |        |         |  | _widthInsideBounds
 * |  |        |         |  | _penaltyWidth
 * |--|        |         |--|
 * |           |            | -y
 * |------------------------|
 *   own half     opposite
 */

PlayingField::PlayingField(FieldSize fieldSize) {
    _size = fieldSize;

    switch (_size) {
        case FieldSize::JRL:
            // This the JRL's permanent playingfield (defined accordning to pre 2012 SPL rules)
            // since there are no separate goal & penalty boxes, both will be defined with the same dimensions
            createField({.fieldLength = 5.95f,
                    .fieldWidth = 3.95f,
                    .centerCircleDiameter = 1.15f,
                    .penaltyCrossDistance = 1.35f,
                    // NOTE: penalty area lines removed, make penalty area == goalbox instead
                    //.penaltyAreaLength = 1.65f,
                    //.penaltyAreaWidth = 2.9f
                    .penaltyAreaLength = 0.6f,
                    .penaltyAreaWidth = 2.2f
            });
            break;
        case FieldSize::HTWK:
            // field for HTWK Leipzig event, May 2014
            createField({.fieldLength = 7.5f, .fieldWidth = 5.f});
            break;
        case FieldSize::TINY:
            // tiny 4x1.8m field for exhibitions
            createField({
                    .fieldLength = 4.f,
                    .fieldWidth = 1.75f,
                    .centerCircleDiameter = 0.8f,
                    .penaltyCrossDistance = 1.1f,
                    .penaltyAreaLength = 0.55f,
                    .penaltyAreaWidth = 1.f,
                    .goalboxAreaLength = 0.55f,
                    .goalboxAreaWidth = 1.f,
                    .goalWidth = 0.56f,
                    .goalPostDiameter = 0.1f,
                    .goalInsideDepth = 0.3f,
            });

            break;
        case FieldSize::SPL:
        default:
            // use function defaults defined in header for current SPL field,
            // according to the Rules 2020 Draft, published on December 16th 2019
            createField(FieldMeasurements());
    }

    createDistVecOfCrosses();

    jsassert(_poles.size() == PoleMAX);
    jsassert(_lines.size() == LineMAX);
    jsassert(_crosses.size() == CrossMAX);
}

PlayingField::PlayingField(const std::string &json_file) {
    using namespace boost::property_tree;
    ptree pt;
    json_parser::read_json(json_file, pt);

    auto &field = pt.get_child("field");
    auto &goal  = pt.get_child("goal");

    float paLength{field.get<float>("penaltyAreaLength")};
    float paWidth{field.get<float>("penaltyAreaWidth")};

    createField({
        .fieldLength = field.get<float>("length"),
        .fieldWidth = field.get<float>("width"),
        .centerCircleDiameter  = field.get<float>("centerCircleDiameter"),
        .penaltyCrossDistance  = field.get<float>("penaltyCrossDistance"),
        .penaltyAreaLength  = paLength,
        .penaltyAreaWidth  = paWidth,
        .goalboxAreaLength  = field.get<float>("goalBoxAreaLength", paLength), // if no goalbox exists, make it congruent with penalty area
        .goalboxAreaWidth  = field.get<float>("goalBoxAreaWidth", paWidth),
        .goalWidth  = goal.get<float>("innerWidth"),
        .goalPostDiameter  = goal.get<float>("postDiameter"),
        .goalInsideDepth  = goal.get<float>("depth"),
        .goalPostHeight  = goal.get<float>("height"),
        .borderStripWidth  = field.get<float>("borderStripWidth"),
        .penaltyCrossSize  = field.get<float>("penaltyCrossSize")
    });

    _size = FieldSize::JSON;
}

PlayingField::~PlayingField() {}

void PlayingField::createField(const FieldMeasurements &fm) {

    // total size of field (playing area + border strips)
    _length = fm.fieldLength + 2 * fm.borderStripWidth;
    _width = fm.fieldWidth + 2 * fm.borderStripWidth;

    // set length and width (of area inside the lines) in meters
    _lengthInsideBounds = fm.fieldLength;
    _widthInsideBounds = fm.fieldWidth;

    _lineWidth = fm.lineWidth;

    // set Goal width (distance between center of goal posts)
    _goalWidth = fm.goalWidth + fm.goalPostDiameter;

    _goalBoxWidth = fm.goalboxAreaWidth;
    _goalBoxLength = fm.goalboxAreaLength;

    _penaltyLength = fm.penaltyAreaLength;
    _penaltyWidth = fm.penaltyAreaWidth;

    // center circle
    _circle.wcs_x = 0.0f;
    _circle.wcs_y = 0.0f;
    _circle.name = Line::CENTER_CIRCLE;
    _circle.wcs_radius = fm.centerCircleDiameter / 2.f;

    _lines.clear();
    _crosses.clear();

    _penaltyCrossDistance = fm.penaltyCrossDistance;
    _penaltyCrossSize = fm.penaltyCrossSize;

    const float x = fm.fieldLength / 2.f;
    const float y = fm.fieldWidth / 2.f;

    // outer borders
    createNewLineDefinition(0.0f * M_PI_F, x, y, -x, y, Line::LEFT);
    createNewLineDefinition(0.0f * M_PI_F, x, -y, -x, -y, Line::RIGHT);
    createNewLineDefinition(0.5f * M_PI_F, x, y, x, -y, Line::TOP);
    createNewLineDefinition(0.5f * M_PI_F, -x, y, -x, -y, Line::BOTTOM);

    // center line
    createNewLineDefinition(0.5f * M_PI_F, 0.0f, y, 0.0f, -y, Line::MIDDLE);

    const float px = x - fm.penaltyAreaLength;
    const float py = fm.penaltyAreaWidth / 2.f;
    const float mx = x - fm.penaltyCrossDistance;

    // opponent's penalty area
    createNewLineDefinition(0.5f * M_PI_F, px, py, px, -py, Line::OPP_PENALTY);
    createNewLineDefinition(0.0f * M_PI_F, x, py, px, py, Line::OPP_PENALTY_LEFT);
    createNewLineDefinition(0.0f * M_PI_F, x, -py, px, -py, Line::OPP_PENALTY_RIGHT);
    createNewLineDefinition(0.0f * M_PI_F, mx, 0.0f, mx, 0.0f, Line::OPP_PENALTY_SHOOTMARK);

    // our penalty area
    createNewLineDefinition(0.5f * M_PI_F, -px, py, -px, -py, Line::OWN_PENALTY);
    createNewLineDefinition(0.0f * M_PI_F, -x, py, -px, py, Line::OWN_PENALTY_LEFT);
    createNewLineDefinition(0.0f * M_PI_F, -x, -py, -px, -py, Line::OWN_PENALTY_RIGHT);
    createNewLineDefinition(0.0f * M_PI_F, -mx, 0.0f, -mx, 0.0f, Line::OWN_PENALTY_SHOOTMARK);

    const float gbx = x - fm.goalboxAreaLength;
    const float gby = fm.goalboxAreaWidth / 2.f;

    // opponent's goalbox area
    createNewLineDefinition(0.5f * M_PI_F, gbx, gby, gbx, -gby, Line::OPP_GOALBOX);
    createNewLineDefinition(0.0f * M_PI_F, x, gby, gbx, gby, Line::OPP_GOALBOX_LEFT);
    createNewLineDefinition(0.0f * M_PI_F, x, -gby, gbx, -gby, Line::OPP_GOALBOX_RIGHT);

    // our goalbox area
    createNewLineDefinition(0.5f * M_PI_F, -gbx, gby, -gbx, -gby, Line::OWN_GOALBOX);
    createNewLineDefinition(0.0f * M_PI_F, -x, gby, -gbx, gby, Line::OWN_GOALBOX_LEFT);
    createNewLineDefinition(0.0f * M_PI_F, -x, -gby, -gbx, -gby, Line::OWN_GOALBOX_RIGHT);

    // goal backing construction
    const float gbCorner_y = fm.goalWidth / 2 + fm.goalPostDiameter / 2;
    const float frontCorner_x = fm.fieldLength / 2;
    const float gbBackCorner_x = frontCorner_x + fm.goalInsideDepth;

    createNewLineDefinition(0, gbBackCorner_x, gbCorner_y, gbBackCorner_x, -gbCorner_y, Line::OPP_GOALBACK_BACK);
    createNewLineDefinition(0, frontCorner_x, gbCorner_y, gbBackCorner_x, gbCorner_y, Line::OPP_GOALBACK_LEFT);
    createNewLineDefinition(0, frontCorner_x, -gbCorner_y, gbBackCorner_x, -gbCorner_y, Line::OPP_GOALBACK_RIGHT);
    createNewLineDefinition(0, -gbBackCorner_x, gbCorner_y, -gbBackCorner_x, -gbCorner_y, Line::OWN_GOALBACK_BACK);
    createNewLineDefinition(0, -frontCorner_x, gbCorner_y, -gbBackCorner_x, gbCorner_y, Line::OWN_GOALBACK_LEFT);
    createNewLineDefinition(0, -frontCorner_x, -gbCorner_y, -gbBackCorner_x, -gbCorner_y, Line::OWN_GOALBACK_RIGHT);

    const float c = fm.centerCircleDiameter / 2.0f;

    // these are the cross definitions
    // field corners
    createNewCrossDefinition(x, y, 1.0f * M_PI_F, 2, Cross::L_OPP_LEFT);
    createNewCrossDefinition(0.0f, y, 1.5f * M_PI_F, 3, Cross::T_LEFT);
    createNewCrossDefinition(-x, y, 1.5f * M_PI_F, 2, Cross::L_OWN_LEFT);
    createNewCrossDefinition(x, -y, 0.5f * M_PI_F, 2, Cross::L_OPP_RIGHT);
    createNewCrossDefinition(0.0f, -y, 0.5f * M_PI_F, 3, Cross::T_RIGHT);
    createNewCrossDefinition(-x, -y, 0.0f * M_PI_F, 2, Cross::L_OWN_RIGHT);
    createNewCrossDefinition(0.0f, c, 0.5f * M_PI_F, 4, Cross::X_LEFT);
    createNewCrossDefinition(0.0f, -c, 0.5f * M_PI_F, 4, Cross::X_RIGHT);

    // opponent penalty area
    createNewCrossDefinition(x, py, 1.0f * M_PI_F, 3, Cross::T_OPP_PENALTY_LEFT);
    createNewCrossDefinition(x, -py, 1.0f * M_PI_F, 3, Cross::T_OPP_PENALTY_RIGHT);
    createNewCrossDefinition(px, py, 1.5f * M_PI_F, 2, Cross::L_OPP_PENALTY_LEFT);
    createNewCrossDefinition(px, -py, 0.0f * M_PI_F, 2, Cross::L_OPP_PENALTY_RIGHT);

    // opponent goalbox area
    createNewCrossDefinition(x, gby, 1.0f * M_PI_F, 3, Cross::T_OPP_GOALBOX_LEFT);
    createNewCrossDefinition(x, -gby, 1.0f * M_PI_F, 3, Cross::T_OPP_GOALBOX_RIGHT);
    createNewCrossDefinition(gbx, gby, 1.5f * M_PI_F, 2, Cross::L_OPP_GOALBOX_LEFT);
    createNewCrossDefinition(gbx, -gby, 0.0f * M_PI_F, 2, Cross::L_OPP_GOALBOX_RIGHT);

    // opponent goal backing construction
    createNewCrossDefinition(x, gbCorner_y, 0.0f * M_PI_F, 3, Cross::T_OPP_GOALBACK_LEFT);
    createNewCrossDefinition(x, -gbCorner_y, 0.0f * M_PI_F, 3, Cross::T_OPP_GOALBACK_RIGHT);
    createNewCrossDefinition(gbBackCorner_x, gbCorner_y, M_PI_F, 2, Cross::L_OPP_GOALBACK_LEFT);
    createNewCrossDefinition(gbBackCorner_x, -gbCorner_y, 0.5f * M_PI_F, 2, Cross::L_OPP_GOALBACK_RIGHT);

    // our penalty area
    createNewCrossDefinition(-x, py, 0.0f * M_PI_F, 3, Cross::T_OWN_PENALTY_LEFT);
    createNewCrossDefinition(-x, -py, 0.0f * M_PI_F, 3, Cross::T_OWN_PENALTY_RIGHT);
    createNewCrossDefinition(-px, py, 1.0f * M_PI_F, 2, Cross::L_OWN_PENALTY_LEFT);
    createNewCrossDefinition(-px, -py, 0.5f * M_PI_F, 2, Cross::L_OWN_PENALTY_RIGHT);

    // our goalbox area
    createNewCrossDefinition(-x, gby, 0.0f * M_PI_F, 3, Cross::T_OWN_GOALBOX_LEFT);
    createNewCrossDefinition(-x, -gby, 0.0f * M_PI_F, 3, Cross::T_OWN_GOALBOX_RIGHT);
    createNewCrossDefinition(-gbx, gby, 1.0f * M_PI_F, 2, Cross::L_OWN_GOALBOX_LEFT);
    createNewCrossDefinition(-gbx, -gby, 0.5f * M_PI_F, 2, Cross::L_OWN_GOALBOX_RIGHT);

    // our goal backing construction
    createNewCrossDefinition(-x, gbCorner_y, 1.0f * M_PI_F, 3, Cross::T_OWN_GOALBACK_LEFT);
    createNewCrossDefinition(-x, -gbCorner_y, 1.0f * M_PI_F, 3, Cross::T_OWN_GOALBACK_RIGHT);
    createNewCrossDefinition(-gbBackCorner_x, gbCorner_y, 1.5f * M_PI_F, 2, Cross::L_OWN_GOALBACK_LEFT);
    createNewCrossDefinition(-gbBackCorner_x, -gbCorner_y, 0.0f * M_PI_F, 2, Cross::L_OWN_GOALBACK_RIGHT);

    // poles
    const float g{(fm.goalWidth + fm.goalPostDiameter) / 2.f};
    const float poleX{x + ((fm.goalPostDiameter - fm.lineWidth) / 2.f)};
    createNewPoleDefinition(poleX, g, fm.goalPostHeight, fm.goalPostDiameter, 0, Pole::OPP_LEFT);
    createNewPoleDefinition(poleX, -g, fm.goalPostHeight, fm.goalPostDiameter, 0, Pole::OPP_RIGHT);
    createNewPoleDefinition(-poleX, g, fm.goalPostHeight, fm.goalPostDiameter, 0, Pole::OWN_LEFT);
    createNewPoleDefinition(-poleX, -g, fm.goalPostHeight, fm.goalPostDiameter, 0, Pole::OWN_RIGHT);

    // goals
    createNewGoalDefinition(_poles[static_cast<int>(Pole::OPP_LEFT)],
            _poles[static_cast<int>(Pole::OPP_RIGHT)],
            fm.goalWidth + fm.goalPostDiameter,
            1);
    createNewGoalDefinition(_poles[static_cast<int>(Pole::OWN_LEFT)],
            _poles[static_cast<int>(Pole::OWN_RIGHT)],
            fm.goalWidth + fm.goalPostDiameter,
            0);

    _outerDiagonal = sqrtf(_length * _length + _width * _width);
    _innerDiagonal = sqrtf(_lengthInsideBounds * _lengthInsideBounds + _widthInsideBounds * _widthInsideBounds);

    // according to Tim's transformation:
    // folgt aus den Verhältnis, dass der Kreis durch die Torpfosten
    // und den mittelpunkt des Strafraums gehen soll
    float centerpointGoaliDefenderCircle_xdiff =
            (_penaltyWidth / 2.0f) * (_penaltyWidth / 2.0f) / (_penaltyLength) - ((_penaltyLength) / 4);
    centerpointGoaliDefenderCircle = {-(_lengthInsideBounds / 2 + centerpointGoaliDefenderCircle_xdiff), 0.0f};
    radiusGoaliDefenderCircle = (_penaltyLength / 2.0f) + centerpointGoaliDefenderCircle_xdiff;
}

Coord PlayingField::getPlayingBoxStart(RobotRole role) {
    Coord p;
    switch (role) {
        case RobotRole::STRIKER:
            p = {_lengthInsideBounds / 6.0f, _widthInsideBounds / 2.0f};
            break;
        case RobotRole::DEFENDER:
            p = {-_lengthInsideBounds / 2.0f, -_widthInsideBounds / 2.0f};
            break;
        case RobotRole::SUPPORTER_DEFENSE:
            p = {-_lengthInsideBounds / 4.0f, -_widthInsideBounds / 4.0f};
            break;
        default:
            p = {-_lengthInsideBounds / 2.0f, -_widthInsideBounds / 2.0f};
            break;
    }
    return p;
}

Coord PlayingField::getPlayinBoxEnd(RobotRole role) {
    Coord p;
    switch (role) {
        case RobotRole::STRIKER:
            p = {_lengthInsideBounds / 2.0f, _widthInsideBounds / 2.0f};
            break;
        case RobotRole::DEFENDER:
            p = {-_lengthInsideBounds / 6.0f, _widthInsideBounds / 2.0f};
            break;
        case RobotRole::SUPPORTER_DEFENSE:
            p = {_lengthInsideBounds / 4.0f, _widthInsideBounds / 4.0f};
            break;
        default:
            p = {_lengthInsideBounds / 2.0f, _widthInsideBounds / 2.0f};
            break;
    }
    return p;
}

Coord PlayingField::getPenaltyMarkPosition(bool opponent) const {
    const auto name = (opponent) ? Line::OPP_PENALTY_SHOOTMARK : Line::OWN_PENALTY_SHOOTMARK;
    const auto &l = _lines[int(name)];
    return {l.start_x, l.start_y};
}

DirectedCoord PlayingField::getReadyPose(int botId, bool hasKickoff) const {
    DirectedCoord p;
    jsassert(0 <= botId && botId < NUM_PLAYERS);
    switch (botId) {
        case 0:
            p.coord = {-_lengthInsideBounds / 2.0f, 0.f};
            break;
        case 1:
            p.coord = {-_lengthInsideBounds / 2.0f + _penaltyCrossDistance - 0.2f, - _goalWidth /4.f};
            break;
        case 2:
            p.coord = {-(_lengthInsideBounds / 2.0f) * 0.5f - 0.3f , _goalBoxWidth/ 4.f};
            break;
        case 3:
            p.coord = {-(_lengthInsideBounds / 2.0f) * 0.3f, - _goalWidth / 1.7f};
            break;
        case 4:
            if (hasKickoff)
                p.coord = {-_circle.wcs_radius, 0.f};
            else
                p.coord = {-_circle.wcs_radius - 0.60f, 0.f};
            break;
        case 5:
            p.coord = {-_circle.wcs_radius/2.f, -_widthInsideBounds/4.f};
            break;
        case 6:
            p.coord = {-_circle.wcs_radius/2.f, _widthInsideBounds/4.f};
            break;
    }
    return p;
}

/*
 *   _lengthInsideBounds
 *   _penaltyLength
 *      -x           x
 * |------------------------|
 * |           |            | y
 * |--|        |         |--|
 * |  |        |         |  | _widthInsideBounds
 * |  |        |         |  | _penaltyWidth
 * |--|        |         |--|
 * |           |            | -y
 * |------------------------|
 *   own half     opposite
 *
 */

bool PlayingField::insidePenaltybox(const Coord &pos) const {
    float errorTolerance = 0.1f;
    // width = from goal to center cirlce, 60cm for example (x)
    // length = from pole to pole, 2.2m for example (y)
    float maxY = _penaltyWidth / 2.0f + errorTolerance;
    bool widthOk = (pos.y >= -maxY) && (pos.y <= maxY);

    float minX = -((_lengthInsideBounds / 2.0f) + errorTolerance);
    float maxX = minX + _penaltyLength;
    bool heightOk = (pos.x >= minX) && (pos.x <= maxX);

    return widthOk && heightOk;
}

// returns default unpenalize pose!
std::vector<DirectedCoord> PlayingField::getUnpenalizedPose() const {
    const float l = _lengthInsideBounds / 2.f - _penaltyCrossDistance;
    const float w = _widthInsideBounds / 2.f;
    return {{-l, w, Rad{-M_PI_2_F}}, {-l, -w, Rad{M_PI_2_F}}};
}

// returns initial Positions vektor[x] = Position id x
std::vector<DirectedCoord> PlayingField::getInitialPose(const int &robot_id, const RobotRole &role, const bool &seven) const {
    if (role == RobotRole::OBSTACLE_AVOIDER){
        const float x{_lengthInsideBounds / 2.f};
        return {{x-5.3f, 0.f}}; // SPL field striker manual placement pose, other fields equivalent distance
    } else {
        const float x{-(_lengthInsideBounds / 2.f)};
        const float y{_widthInsideBounds / 2.f};
        const Angle a = Rad{M_PI_2_F};
        static const std::vector<DirectedCoord> defaultPoses {
            {x * 0.87f,  y, -a},
            {x * 0.71f, -y,  a},
            {x * 0.50f,  y, -a},
            {x * 0.35f, -y,  a},
            {x * 0.17f,  y, -a},
            {x * 0.17f, -y,  a}};

        if (seven || robot_id > 4) {
            // poses for 7vs7 challenge
            static const std::vector<DirectedCoord> defaultPoses7 {
                {x * 0.87f,  y, -a},
                {x * 0.71f, -y,  a},
                {x * 0.63f,  y, -a},
                {x * 0.48f, -y,  a},
                {x * 0.40f,  y, -a},
                {x * 0.24f, -y,  a},
                {x * 0.17f,  y, -a}};

            return {defaultPoses7.at(robot_id)};
        }
        return {defaultPoses.at(robot_id)};
    }
}

std::vector<DirectedCoord> PlayingField::getObstaclePositionsObstacleChallenge(){
    std::vector<DirectedCoord> result;
    static std::default_random_engine random_engine = std::default_random_engine();

    //positions as robots are in opposite team(so in their own half on a spl field)
    const float fieldhalf_length{_lengthInsideBounds / 2.f};
    std::vector<float> possible_xs= {-(fieldhalf_length-0.6f),-(fieldhalf_length-1.8f),-(fieldhalf_length-3.25f)};
    std::shuffle(std::begin(possible_xs), std::end(possible_xs), random_engine);
    // y =0
    result.push_back({possible_xs.at(0), 0.0f, Rad{0.0f}});

    // y= + or - 0.2
    if (rand()%2 ==0){
        result.push_back({possible_xs.at(1), 0.2f, Rad{0.0f}});
    }
    else{
        result.push_back({possible_xs.at(1), -0.2f,Rad{0.0f}});
    }

    // y= 0.45 and -0.45
    result.push_back({possible_xs.at(2), -0.45f, Rad{0.0f}});
    result.push_back({possible_xs.at(2), +0.45f, Rad{0.0f}});

    return result;
}

std::vector<DirectedCoord> PlayingField::getObstaclePositionsPassingChallenge(){
    std::vector<DirectedCoord> result;
    static std::default_random_engine random_engine = std::default_random_engine();

    std::vector<float> possible_ys= {(2.0f-0.6f),(2.0f-1.2f),(2.0f-1.8f)};
    std::shuffle(std::begin(possible_ys), std::end(possible_ys), random_engine);
    result.push_back({0.0f,possible_ys.at(0), Rad{0.0f}});
    result.push_back({0.0f,possible_ys.at(1), Rad{0.0f}});
    
    return result;
}

std::array<Coord, 4> PlayingField::getCorner() {
    //const Cross name
    LandmarkCross cross1 = getCross(Cross::L_OWN_RIGHT);
    LandmarkCross cross2 = getCross(Cross::L_OWN_LEFT);
    LandmarkCross cross3 = getCross(Cross::L_OPP_RIGHT);
    LandmarkCross cross4 = getCross(Cross::L_OPP_LEFT);
    return {Coord(cross1.wcs_x, cross1.wcs_y),
            Coord(cross2.wcs_x, cross2.wcs_y),
            Coord(cross3.wcs_x, cross3.wcs_y),
            Coord(cross4.wcs_x, cross4.wcs_y)};
}

std::array<Coord, 4> PlayingField::getGoalSpot() {
    bool opponent = true;
    Coord ownPenaltySpot = getPenaltyMarkPosition(!opponent);
    Coord oppPenaltySpot = getPenaltyMarkPosition(opponent);

    LandmarkCross ownGoalLeft = getCross(Cross::L_OWN_GOALBOX_LEFT);
    LandmarkCross ownGoalRight = getCross(Cross::L_OWN_GOALBOX_RIGHT);

    LandmarkCross oppGoalLeft = getCross(Cross::L_OPP_GOALBOX_LEFT);
    LandmarkCross oppGoalRight = getCross(Cross::L_OPP_GOALBOX_RIGHT);

    return {Coord(ownPenaltySpot.x, ownGoalLeft.wcs_y),
            Coord(ownPenaltySpot.x, ownGoalRight.wcs_y),
            Coord(oppPenaltySpot.x, oppGoalLeft.wcs_y),
            Coord(oppPenaltySpot.x, oppGoalRight.wcs_y)};
}

float PlayingField::penaltyBoxToFieldBorderLength() const { return (_width - _penaltyWidth) / 2; }

float PlayingField::penaltyBoxToMiddleLineLength() const { return (_length / 2) - _penaltyLength; }

// creating functions used in constructor to fill the objekt with live

void PlayingField::createNewLineDefinition(const float alpha, const float startX, const float startY, const float endX,
        const float endY, const Line lineType) {
    jsassert(_lines.size() == static_cast<size_t>(lineType));
    LandmarkLine line;

    line.equationNormal = vectorCrossProductUnNormed({startX, startY, 1.f}, {endX, endY, 1.f});
    line.equation = getHesseNormalFormOfLine(line.equationNormal);

    /* line.wcs_alpha = alpha; */

    line.start_x = startX;
    line.start_y = startY;
    line.end_x = endX;
    line.end_y = endY;

    line.name = lineType;

    _lines.push_back(line);
}

void PlayingField::createNewCrossDefinition(
        const float x, const float y, const float alpha, const int degree, const Cross crossType) {
    //std::cout << _crosses.size() << ","<< static_cast<size_t>(crossType)<<std::endl;
    jsassert(_crosses.size() == static_cast<size_t>(crossType));
    LandmarkCross cross;

    cross.equation = {x, y, 1.f};
    cross.wcs_x = x;
    cross.wcs_y = y;
    cross.wcs_alpha = alpha;
    cross.degree = degree;
    cross.name = crossType;

    _crosses.push_back(cross);
}

LandmarkPole PlayingField::createNewPoleDefinition(
        const float x, const float y, const float height, const float width, const int color, const Pole poleType) {
    jsassert(_poles.size() == static_cast<size_t>(poleType));

    LandmarkPole pole;
    pole.wcs_x = x;
    pole.wcs_y = y;
    pole.wcs_width = width;
    pole.wcs_height = height;
    pole.color = color;
    pole.name = poleType;
    pole.equation = {x, y, 1.f};

    _poles.push_back(pole);

    return pole;
}

void PlayingField::createNewGoalDefinition(
        const LandmarkPole &left, const LandmarkPole &right, const float poleDistance, bool opponent) {
    LandmarkGoal goal;
    goal.left = left;
    goal.right = right;
    goal.wcs_pole_distance = poleDistance;
    goal.color = left.color;

    if (opponent) {
        _goals.first = goal;
    } else {
        _goals.second = goal;
    }
}

void PlayingField::createDistVecOfCrosses() {
    for (auto &c1 : _crosses) {
        c1.distances.clear();

        for (const auto &c2 : _crosses) {
            float dist = Coord(c1.wcs_x, c1.wcs_y).dist(Coord(c2.wcs_x, c2.wcs_y));
            c1.distances.push_back(dist);
        }
        jsassert(c1.distances.size() == CrossMAX);
        jsassert(c1.distances[static_cast<int>(c1.name)] < 0.01f);
    }
}

// geting functions ... to get specific objects

std::vector<LandmarkLine> PlayingField::getLines() const { return _lines; }

// get Crosses/Edges of a given degree
std::vector<LandmarkCross> PlayingField::getCrosses(const int &degree) const {
    std::vector<LandmarkCross> result;
    std::copy_if(_crosses.begin(), _crosses.end(), std::back_inserter(result), [&](const LandmarkCross &c) {
        return c.degree == degree;
    });
    return result;
}

std::vector<LandmarkCross> PlayingField::getLCrosses() const { return getCrosses(2); }

std::vector<LandmarkCross> PlayingField::getTCrosses() const { return getCrosses(3); }

std::vector<LandmarkCross> PlayingField::getXCrosses() const { return getCrosses(4); }

LandmarkCross PlayingField::getCross(const Cross name) const {
    int i = static_cast<int>(name);
    // check whether someone is doing something nasty
    jsassert(i >= 0);
    jsassert(i < static_cast<int>(_crosses.size()));

    // array indices are equal to enum numbers (guaranteed by assert)
    return _crosses[i];
}

// get all crosses
std::vector<LandmarkCross> PlayingField::getAllCrosses() const {
    std::vector<LandmarkCross> result;
    std::vector<LandmarkCross> deg2;
    std::vector<LandmarkCross> deg3;
    std::vector<LandmarkCross> deg4;

    deg2 = getTCrosses();
    deg3 = getLCrosses();
    deg4 = getXCrosses();

    result.insert(result.end(), deg2.begin(), deg2.end());
    result.insert(result.end(), deg3.begin(), deg3.end());
    result.insert(result.end(), deg4.begin(), deg4.end());

    return result;
}

std::vector<LandmarkPole> PlayingField::getPoles() const { return _poles; }

std::vector<LandmarkOrientation> PlayingField::getDistsAndAngles(const DirectedCoord &from, int type) {

    std::vector<DirectedCoord> landmark_positions;

    // len(L)     = 8, type 0
    // len(T)     = 6, type 1
    // len(X)     = 2, type 2
    // len(poles) = 4, type 3

    std::vector<LandmarkOrientation> ret;

    // Crosses
    for (const auto &c : _crosses) {
        LandmarkOrientation lo;
        memset(&lo, 0, sizeof(lo));
        if (c.degree == 3) {
            lo.landmark = 1;
        } else if (c.degree == 4) {
            lo.landmark = 2;
        } else {
            lo.landmark = 0;
        }

        landmark_positions.emplace_back(c.wcs_x, c.wcs_y);
        ret.push_back(lo);
    }

    // Poles
    LandmarkOrientation lo;
    memset(&lo, 0, sizeof(lo));
    lo.landmark = 3;
    for (const auto &p : _poles) {
        landmark_positions.emplace_back(p.wcs_x, p.wcs_y);
        ret.push_back(lo);
    }

    auto rt = ret.begin();
    for (const auto &lp : landmark_positions) {
        // translate this target wcs position to rcs, according to the
        // own position in the wcs
        DirectedCoord guess = lp.toRCS(from);
        rt->rcs_dist = guess.coord.dist();
        rt->rcs_angle = guess.coord.angle().rad();
    }

    return ret;
}

std::vector<DirectedCoord> PlayingField::kick_off_position(SetPlay set_play) {
    switch (set_play){
    case SetPlay::NONE:
        return {DirectedCoord{}};
    case SetPlay::GOAL_KICK:
        return {{_lengthInsideBounds/2.f -_goalBoxLength, -_goalBoxWidth/2.f},
            {_lengthInsideBounds/2.f -_goalBoxLength, _goalBoxWidth/2.f}};
    case SetPlay::CORNER_KICK:
        return {{_lengthInsideBounds/2.f, -_widthInsideBounds},
            {_lengthInsideBounds/2.f, _widthInsideBounds}};
    case SetPlay::PENALTY_KICK:
        return {{_lengthInsideBounds/2.f -_penaltyCrossDistance, 0.f}};          
    default:
        return {};//No positions known
    }
}

bool PlayingField::ball_outside_centercircle(Coord ball) {
    //LOG_DEBUG << "BALL: " << ball;
    float distance_from_center = sqrt(ball.x * ball.x + ball.y * ball.y);
    //LOG_DEBUG << "DISTANCE FROM CENTER: " << distance_from_center << "WCS_RADIUS: "<< _circle.wcs_radius;
    return (distance_from_center > _circle.wcs_radius);
}

DirectedCoord PlayingField::goaliDefenderPosition(Angle theta) {
    DirectedCoord directedcenterpointCircle = {centerpointGoaliDefenderCircle, theta};
    /*Coord direction = (ball-centerpointGoal)/(ball-centerpointGoal).norm2();
    LOG_DEBUG <<"direction"<<direction;
    Coord way = ((ball-centerpointGoal)/(ball-centerpointGoal).norm2())*radiusGoaliDefenderCircle;
    LOG_DEBUG <<"way"<<way;
    Coord point = (centerpointGoal + way);
    LOG_DEBUG <<"point"<<point;
    LOG_DEBUG << "point to ball"<< point.lookAt(ball).angle.rad; 
    LOG_DEBUG << "ball to point"<< ball.lookAt(point).angle.rad; */
    return directedcenterpointCircle.walk(DirectedCoord(radiusGoaliDefenderCircle, 0.0f, Rad{0.0f}));
}

Angle PlayingField::getCurrentThetaGoali(DirectedCoord bot_pos) {
    return centerpointGoaliDefenderCircle.lookAt(bot_pos.coord).angle;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
