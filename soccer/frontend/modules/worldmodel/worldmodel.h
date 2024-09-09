#pragma once
#include "ballmotionfilter.h"
#include "ballfilter.h"
#include "framework/rt/endpoints/input.h"
#include "gamecontrol_generated.h"
#include "teamball.h"

#include <framework/rt/module.h>

#include <representations/vision/visiondefinitions.h>
#include <representations/bembelbots/constants.h>
#include <representations/blackboards/settings.h>
#include <representations/blackboards/worldmodel.h>
#include <representations/teamcomm/teammessage.h>
#include <representations/motion/body_state.h>
#include <representations/flatbuffers/flatbuffers.h>

#include <array>
#include <vector>
/**
 * @section coordsys Coordinate Systems
 * @image html coordsys.png
 * @image latex coordsys.png "coordinate systems used in the framework" width=.8\textwidth
 * @subsection worldcoordsys World Coordinate Frame
 * A \b right \b handed coordinate system \b centerned \b at \b the \b middle of the playing field,
 * the \b x-axis \b points \b towards \b the \b opponent's \b goal, the z-axis points upwards.
 * If an angle alpha is specified, this is always a rotation around the z-axis, measured from x-axis (alpha=0)
 * to y-axis (e.g. alpha=pi/2 rotates from 1,0 to 0,1). \f$ \vec p_{rotated} = R(\alpha) * \vec p\f$ with \f$R = \left(\begin{array}{cc} cos \alpha & -sin \alpha\\ sin \alpha & cos \alpha \end{array}\right)\f$.
 * @subsection robotcoordsys Robot Coordinate Frame
 * Same...
 */

class PlayingField;
class SettingsBlackboard;

/**
 * The world model provides information about the world seen by the robots.
 *
 * The world model is the interface to robot an ball pose for all upper level
 * behaviour. The WM provides:
 * - the estimated WCS position of our 3 robots, see getRobotPoseWcs() /
 * getMyRobotPoseWcs(), obtained from last pose estimate for the own bot and
 * from last received pose estimate for other bots of our team
 * - the most likely ball pose in WCS coordinates
 */
class WorldModel : public rt::Module {
public:

    static constexpr float BALL_SIZE = CONST::ball_size;
    static constexpr float ROBOT_SIZE = CONST::robot_size;

    /**
     * Default Constructor.
     */
    WorldModel();
    
    void connect(rt::Linker &) override;
    void setup() override;
    void process() override;

    //std::vector<Coord> situationBasedBallPositions();

private:
    rt::Context<SettingsBlackboard> settings;
    rt::Context<PlayingField> playingfield;
    rt::Input<TeamMessage, rt::Snoop> teamMessage;
    rt::Input<bbapi::LocalizationMessageT, rt::Require> loca;
    rt::Input<VisionResultVec, rt::Require> visionResults;
    rt::Input<BodyState> body;
    rt::Input<bbapi::GamecontrolMessageT, rt::Listen> gamecontrol;
    //rt::Input<bbapi::GamecontrolMessageT, rt::NonBlocking> gamecontrol;

    void updateRobotPose();
    void updateTeamBall();
    void insertDetectedRobot(Robot newRobot);
    void updateDetectedRobots();
    void useBodyStiffnessforObstacleDetection();
    void runGoaliballMotionFilter(Coord ball_rcs, Ball &ballWcs);
    void useVisionResults(const DirectedCoord &myPos);

    WorldModelBlackboard world;
    rt::Output<Snapshot<WorldModelBlackboard>> output;

    Robot *robot;
    
    /*
     * JonathansBallMotionFilter only works for Goali 
     * when not moving head and standing on goal line
     */
    std::shared_ptr<JonathansBallMotionFilter> goaliballMotionFilter;
    std::shared_ptr<BallFilter> expBallposFilter;
    std::shared_ptr<TeamBall> teamball;

    TimestampMs _lastWmDebugToConsole;
};

std::ostream &operator<<(std::ostream &s,
                         const WorldModel &wm); ///< stream output function

// vim: set ts=4 sw=4 sts=4 expandtab:
