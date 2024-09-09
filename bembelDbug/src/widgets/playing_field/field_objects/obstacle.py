from typing import List
import base64
import struct

import math
from PyQt5 import QtGui
from PyQt5.QtCore import Qt, QPointF
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QFont
from PyQt5.QtGui import QPainter
from utils.parseFrameworkClasses import RobotData
from widgets.playing_field.field_objects.field_object import FieldObject
from widgets.playing_field.spl_receiver import BembelSPLMessage

class Obstacles(FieldObject):
    def __init__(self,name, team_id=None, robot_id=None):
        super().__init__(name, team_id=team_id, robot_id=robot_id)
        self.obstacles=[]    
        self.visible_by_default = False
    
    def drawWCS(self, painter: QPainter):
        for (x, y ) in self.obstacles:
            pen = QtGui.QPen()
            color = QtGui.QColor()

            painter.save()
            color.setRgb(242, 0, 187, 150)
            pen.setColor(color)
            pen.setCapStyle(Qt.RoundCap)
            pen.setWidth(300)
            painter.setPen(pen)
            painter.drawPoint(QPointF(x,y)*1000)
            
            painter.restore()

class DetectedRobots(Obstacles):
    def __init__(self, team_id=None, robot_id=None):
        super().__init__("detected robots", team_id=team_id, robot_id=robot_id)
        self.BlackboardSymbols = ["WorldModel.detectedRobots"]

    def setFromString(self, s, _):
        self.obstacles = []
        SIZE_ROBOT = RobotData.SIZE
        s = base64.b64decode(s)
        if len(s) % SIZE_ROBOT != 0:
            print("wrong size received for robots!")
            return
        count = int(len(s) / SIZE_ROBOT)
        offset = 0
        for i in range(count):
            robo = RobotData(s[offset: offset + SIZE_ROBOT])
            offset += SIZE_ROBOT

            self.obstacles.append((robo.posX, robo.posY))

        self.available = True

    def setFromSPLMessage(self, message):

        if not isinstance(message, BembelSPLMessage):
            return

        self.obstacles = []

        for o in message.reactivewalk_obstacles:
            self.obstacles.append((o[0], o[1]))

        self.available = True

