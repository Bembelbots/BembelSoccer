from typing import List

import math
import numpy as np
from PyQt5 import QtGui
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QFont
from PyQt5.QtGui import QPainter
from PyQt5.QtCore import QPointF, QLineF, QRectF
from PyQt5.QtGui import QPen

from widgets.playing_field.field_objects.field_object import FieldObject
from widgets.playing_field.team_viewer_drawer_widget import TeamViewerDrawerWidget
from widgets.playing_field.spl_receiver import BembelSPLMessage

class ReactiveWalkTarget(FieldObject):
    DEFAULT_SIZE = 35
    def __init__(self, team_id=None, robot_id=None):
        super().__init__("ReactiveWalk: goal target", team_id=team_id, robot_id=robot_id)
        self.confidence = 1
        self.BlackboardSymbols = ["ReactiveWalkBlackboard.debug_target_dribble"]
        
    @property
    def confidence(self) -> float:
        return self._confidence

    @confidence.setter
    def confidence(self, confidence: float):
        self._confidence = confidence

    def setFromString(self, s, _):
        s = s.split(",")
        x,y = float(s[0]),float(s[1])
        self._position = (x,y)
        self.available = True

    def setFromSPLMessage(self, message):

        if not isinstance(message, BembelSPLMessage):
            return

        self._position = (message.reactivewalk_goaltarget_pos[0], message.reactivewalk_goaltarget_pos[1])
        self.available = True

    def drawWCS(self, painter: QPainter):
        if self.position[0] == 0 and self.position[1] == 0:
            return
        pen = QtGui.QPen()
        color = QtGui.QColor()

        color.setRgb(255, 165, 0, 150)
        pen.setColor(color)
        pen.setCapStyle(Qt.RoundCap)
        pen.setWidth(250)
        painter.setPen(pen)
        (x,y) = self.position
        painter.drawPoint(QPointF(x,y)*1000)
            
    # def drawWCSText(self, drawer):
    #     pos = QtCore.QPointF(self.position[0]*1000, self.position[1]*1000)
    #     drawer.drawTextAtPoint("target_approach", pos.x(), pos.y(), 100)


class ApproachWcsTarget(FieldObject):
    DEFAULT_SIZE = 35
    def __init__(self, team_id=None, robot_id=None):
        super().__init__("ReactiveWalk: walk target", (0,0,0), team_id=team_id, robot_id=robot_id)
        self.confidence = 1
        self.BlackboardSymbols= ["ReactiveWalkBlackboard.debug_target_approach_wcs"]
        
    @property
    def confidence(self) -> float:
        return self._confidence

    @confidence.setter
    def confidence(self, confidence: float):
        self._confidence = confidence

    def setFromString(self, s, _):
        s = s.split(",")
        s2 = s[1].split("@")
        self._position = (float(s[0]),float(s2[0]), float(s2[1]))
        self.available = True

    def setFromSPLMessage(self, message):

        if not isinstance(message, BembelSPLMessage):
            return

        self._position = (message.reactivewalk_walktarget_pos[0], message.reactivewalk_walktarget_pos[1],
            message.reactivewalk_walktarget_alpha)
        self.available = True
    
    def drawWCS(self, painter: QPainter):
        if self.position[0] == 0 and self.position[1] == 0:
            return
        rx, ry = self.parent.position
        rx *= 1000
        ry *= 1000

        self._orientation = self.position[2]
        (px, py) = self.position[0:2]
        px *= 1000
        py *= 1000
        robot_size = 300
        half_size = robot_size / 2
        startAngle = 30 * 16
        spanAngle = 360 * 16

        color = QtGui.QColor()
        color.setRgb(153, 153, 255, 180)
        pen_size = 20
        pen = QPen(color, pen_size)
        brush = QtGui.QBrush(color)

        painter.setPen(pen)

        robot = QRectF(px - half_size, py- half_size, robot_size, robot_size)

        painter.drawLine(QLineF(rx,ry,px,py))

        painter.drawArc(robot, startAngle, spanAngle)

        painter.setBrush(brush)

        painter.save()
        transform = painter.transform()
        transform.translate(px,py)
        #Robot orientattion
        #                           +pi/2
        #  -180/180 bzw. -pi/+pi    < v^>  0
        #                           -pi/2

        transform.rotate(np.degrees(self._orientation))
        painter.setTransform(transform)

        poly = QtGui.QPolygonF()
        poly.append(QPointF(robot.width() / 2 - 1, 0))
        poly.append(QPointF(0, robot.width() / 4 - 1))
        poly.append(QPointF(0, -robot.width() / 4 + 1))
        painter.drawPolygon(poly)

        # draw string
        text = "target_approach"
        painter.restore()
    
    # def drawWCSText(self, drawer):
    #     pos = QtCore.QPointF(self.position[0]*1000, self.position[1]*1000)
    #     drawer.drawTextAtPoint("target_approach", pos.x(), pos.y(), (0,100))

