from PyQt5.QtCore import *
from PyQt5.QtGui import *
import numpy as np
import time
import base64
from widgets.playing_field.field_objects.field_object import FieldObject
from utils.parseFrameworkClasses import BallData

# max ball age, in seconds
MAX_BALL_AGE = 3

class Ball(FieldObject):

    def __init__(self, name, team_id, robot_id):
        super().__init__(name)
        self._message = None
        self.team_id = team_id
        self.robot_id = robot_id
        self.ball_last_seen = -1000
        self.BlackboardSymbols = ["WorldModel.allBallPoseWcs"]

    @property
    def ballAge(self):
        return max(time.time()-self.ball_last_seen, 0)

    def setFromString(self, s, _):
        s = base64.b64decode(s)

        if len(s) % BallData.SIZE != 0:
            print("wrong size received for balls!")
            return

        count = int(len(s) / BallData.SIZE)
        if count < self.robot_id-1:
            print("ball for robot id " + str(self.robot_id) + " was not in sent ball data.")
            return

        offset = BallData.SIZE * (self.robot_id-1)

        ball = BallData(s[offset: offset + BallData.SIZE])

        if ball.ballId != self.robot_id-1:
            print("ball for robot id " + str(self.robot_id) + " was not in sent data packet.")
            return

        if self.parent == None:
            print("No robot for ball specified!")
            return

        self.ball_last_seen = time.time()-(self.parent.last_received_timestamp-ball.message.timestamp)/1000

        self._position = (ball.posX, ball.posY)

        self.available = self.parent.available

    def setFromSPLMessage(self,message):
        if message.team_num != self.team_id or message.player_num != self.robot_id:
            return
        
        rcs_position = (message.ball_pos[0]/1000, message.ball_pos[1]/1000)
        self._position = self._transform_from_rcw_to_wcs(rcs_position)

        self.ball_last_seen = time.time()-message.ball_age


        self.available = self.parent.available

    def drawWCS(self, painter):

        if self.parent == None:
            print("No robot for ball specified!")
            return

        if self.ballAge > MAX_BALL_AGE:
            return        


        rx, ry = self.parent.position
        robot_pos = QPointF(rx*1000, ry*1000)

        bx, by = self.position
        ball_pos = QPointF(bx*1000, by*1000)

        ballColor = QColor(int(255*(1-self.ballAge/MAX_BALL_AGE)), 0, 0)

        pen_size = 10
        painter.setPen(QPen(ballColor, pen_size, Qt.SolidLine))
        painter.setBrush(QBrush(ballColor))

        bsize = 60

        # draw ball
        painter.drawEllipse(ball_pos, bsize, bsize)

        # draw line connecting ball and robot
        painter.drawLine(robot_pos, ball_pos)
