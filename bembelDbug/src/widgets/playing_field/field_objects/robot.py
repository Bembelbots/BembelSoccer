from PyQt5.QtCore import *
from PyQt5.QtGui import *
import base64
import numpy as np
import time
from PyQt5 import QtCore, QtGui
from PyQt5.QtCore import QPointF, QRect
from PyQt5.QtGui import QColor, QPainter, QPen
from utils.constants import Constants

from widgets.playing_field.field_objects.field_object import FieldObject
from widgets.playing_field.team_viewer_drawer_widget import TeamViewerDrawerWidget
from utils.parseFrameworkClasses import RobotData
from widgets.playing_field.spl_receiver import BembelSPLMessage
from utils.coords import DirectedCoord

# how long to render robots when no new packet is received, in seconds
ROBOT_TIMEOUT = 15

# size of robot (in mm on field)
ROBOT_RENDER_SIZE = 250

WM_SYM_ROBOTS = "WorldModel.allRobots"
WM_SYM_MY_ROLE = "WorldModel.myRole"
WM_SYM_NEAREST2BALL = "WorldModel.iAmNearestToOwnBall"

class Robot(FieldObject):

    def __init__(self, name, team_id, robot_id, fieldconfig, own_team_id=None):
        super().__init__(name, team_id=team_id, robot_id=robot_id)
        self.alpha = 0
        self.team_id = team_id
        self.robot_id = robot_id
        self.fieldconfig = fieldconfig
        self.own_team_id = own_team_id
        if own_team_id == None:
            self.own_team_id = Constants.TEAM_ID
        self.available = False
        self.fallen = False
        self._last_update = 0
        self.orientation = 0
        self.is_nearest_to_ball = False
        self.last_received_timestamp = 0
        self.nearest_to_ball = False

        self.gtposition = None
        self.gtorientation = None
        self.role = -1

        self._penalized = False             # true if the robot is penalized
        self.penalizedPosition = DirectedCoord()  # position to draw the robot in if penalized
                                            # this is calculated in the current field config
        self.penalizedTimeRemaining = ""    # string describing the time remaining penalized
        self.penaltySpot = -1               # the penalty spot id given by the field config

        self.BlackboardSymbols = [WM_SYM_ROBOTS, WM_SYM_NEAREST2BALL, WM_SYM_MY_ROLE]

    def setFromString(self, s, arg):
        if arg == WM_SYM_ROBOTS:
            s = base64.b64decode(s)

            if len(s) % RobotData.SIZE != 0:
                print("wrong size received for robots!")
                return

            count = int(len(s) / RobotData.SIZE)
            if count < self.robot_id-1:
                print("robot id " + str(self.robot_id) + " was not in sent robot data.")
                return

            offset = RobotData.SIZE * (self.robot_id-1)

            robo = RobotData(s[offset: offset + RobotData.SIZE])

            if robo.robotId != self.robot_id-1:
                print("robot id " + str(self.robot_id) + " was not in sent data packet.")
                return

            self.orientation = robo.alpha
            self.confidence = robo.confidence
            self._position = (robo.posX, robo.posY)
            self.fallen = float(robo.fallenSince) != -1

            self.available = robo.active or self.available

            self.last_received_timestamp = robo.message.timestamp

            self._last_update = time.time()

        elif arg == WM_SYM_NEAREST2BALL:

            if s == "yes":
                self.is_nearest_to_ball = True
            elif s == "no":
                self.is_nearest_to_ball = False
            else:
                print("Unknown string " + s)

        elif arg == WM_SYM_MY_ROLE:
            self.role = int(s)

    @property
    def penalized(self):
        return self._penalized

    @penalized.setter
    def penalized(self, value):
        if value == self._penalized:
            return

        if value:
            self.penaltySpot, self.penalizedPosition = self.fieldconfig.getNextPenaltySpot(self.team_id)
        else:
            self.fieldconfig.freePenaltySpot(self.team_id, self.penaltySpot)

        self._penalized = value

    def setFromSPLMessage(self,message):
        self._position = (message.position[0]/1000, message.position[1]/1000)
        self.orientation = message.position[2]
        self.fallen = message.fallen

        if message.team_num != self.own_team_id:
            self._position = (self._position[0] * -1, self._position[1] * -1)
            self.orientation = 2*np.pi - self.orientation

        if isinstance(message, BembelSPLMessage):
            self.is_nearest_to_ball = message.is_nearest_to_ball
            self.role = message.role
            self.setText("{num} - {name} \t ({bat}%)".format(
                num = message.player_num,
                name = message.name,
                bat = message.battery
            ))

        # confidence unknown
        self.confidence = -1
        self.available = True
        self._last_update = time.time()

    def handleGCMessage(self, message):
        if not self.available:
            return

        for t in message.teams:
            if t.teamNumber != self.team_id:
                continue
            robotinfo = t.robots[self.robot_id-1]

            self.penalized = robotinfo.penalty != None
            self.penalizedTimeRemaining = robotinfo.penaltyTimeRemaining

    def setManualPosition(self, posX, posY, orientation):
        self._position = (posX, posY)
        self.orientation = orientation
        self._last_update = time.time() + 10000000
        self.available = True
        self.visible = True

    def drawWCS(self, painter):

        if((time.time() - self._last_update) > ROBOT_TIMEOUT):
            self.available = False
            return

        if not self.penalized:
            if self.gtposition is not None:
                robot_pos = QtCore.QPointF(self.gtposition[0]*1000, self.gtposition[1]*1000)
                robot_orientation = self.gtorientation
            else:
                robot_pos = QtCore.QPointF(self.position[0]*1000, self.position[1]*1000)
                robot_orientation = self.orientation
        else:
            robot_pos = QtCore.QPointF(self.penalizedPosition.x*1000, self.penalizedPosition.y*1000)
            robot_orientation = self.penalizedPosition.a

        half_size = ROBOT_RENDER_SIZE / 2.0

        robot_color = QColor(128, 128, 255)
        if(self.team_id != self.own_team_id):
            robot_color = QColor(255, 128, 128)
        elif self.is_nearest_to_ball:
            robot_color = QColor(0,255,0)


        # draw robot
        painter.setPen(QPen(robot_color, 30, Qt.SolidLine))

        painter.setBrush(robot_color)
        if self.fallen:
            painter.setBrush(Qt.NoBrush)

        painter.drawEllipse(robot_pos, half_size, half_size)

        # draw robot rotation
        painter.setPen(QPen(robot_color, 50, Qt.SolidLine))

        painter.save()
        transform = painter.transform()
        transform.translate(robot_pos.x(), robot_pos.y())
        painter.setTransform(transform)
        painter.rotate(np.degrees(robot_orientation))

        painter.drawLine(0,0,250,0)
        painter.restore()

    def drawWCSText(self, drawer):
        name = str(self.robot_id)
        robot_pos = QtCore.QPointF(self.position[0]*1000, self.position[1]*1000)
        if self.penalized:
            robot_pos = QtCore.QPointF(self.penalizedPosition.x*1000, self.penalizedPosition.y*1000)
            drawer.drawTextAtPoint(self.penalizedTimeRemaining, robot_pos.x(), robot_pos.y(), (0,-250))

        drawer.drawTextAtPoint(name, robot_pos.x(), robot_pos.y(), (0,0), 160)
        if 0 < self.role < len(Constants.ROBOT_ROLES):
            drawer.drawTextAtPoint(Constants.ROBOT_ROLES[self.role], robot_pos.x(), robot_pos.y(), (0,250), 200)
        #else:
            #print("Role %i is not defined"%(self.role))

