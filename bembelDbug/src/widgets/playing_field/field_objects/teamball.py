from typing import List

import base64
import struct

import math
from PyQt5 import QtGui
from PyQt5.QtCore import Qt, QPointF
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QFont
from PyQt5.QtGui import QPainter
from utils.parseFrameworkClasses import BallData
from widgets.playing_field.field_objects.field_object import FieldObject
from widgets.playing_field.spl_receiver import BembelSPLMessage


class TeamBall(FieldObject):
    def __init__(self, team_id=None, robot_id=None):
        super().__init__("teamball", team_id=team_id, robot_id=robot_id)
        self._confidence = 0.0
        self.BlackboardSymbols = ["WorldModel.teamball"]
        
    @property
    def confidence(self) -> float:
        return self._confidence

    @confidence.setter
    def confidence(self, confidence: float):
        self._confidence = confidence
    
    def setFromString(self,s, _):
        s = base64.b64decode(s)
        teamballdata = BallData(s)
        self._position = (teamballdata.posX, teamballdata.posY)
        self.confidence = teamballdata.posConfidence 
        self.available = True
        #print("receive Teamball at: ", self._position, "with confidence", self.confidence)

    def setFromSPLMessage(self, message):

        if not isinstance(message, BembelSPLMessage):
            return

        self._position = (message.teamball_pos[0], message.teamball_pos[1])
        self.confidence = message.teamball_confidence
        self.available = True
        #print("receive Teamball at: ",self._position, "with confidence", self.confidence)

    def drawWCS(self, painter: QPainter):

        # do not draw, if confidence 0
        if(self._confidence <= 0):
            return

        pen = QtGui.QPen()
        color = QtGui.QColor()

        painter.save()
        color.setRgb(255, 255, 30,(int)(120*(self._confidence)))
        pen.setColor(color)
        pen.setCapStyle(Qt.RoundCap)
        pen.setWidth(200 * self._scale)
        painter.setPen(pen)
        painter.drawPoint(QPointF(self.position[0], self.position[1])*1000)
        
        painter.restore()
