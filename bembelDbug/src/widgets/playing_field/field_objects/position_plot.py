from typing import List

import math
import numpy as np
from PyQt5 import QtGui
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QFont
from PyQt5.QtGui import QPainter
from PyQt5.QtCore import QPointF, QLineF
from PyQt5.QtCore import QRect
from PyQt5.QtGui import QPen

from widgets.playing_field.field_objects.field_object import FieldObject
from widgets.playing_field.team_viewer_drawer_widget import TeamViewerDrawerWidget

class PositionPlot(FieldObject):
    DEFAULT_SIZE = 35

    def onVisibleChanged(self, visible):
        self.positions.clear()

    def __init__(self):
        super().__init__("Position Plot")
        self.positions = []
        self.BlackboardSymbols = ["BehaviorBlackboard.bot_pos"]
        self.visible_by_default = False

    def setFromString(self, s, _):        
        s = s.split(",")
        s2 = s[1].split("@")
        pos = np.array([float(s[0]),float(s2[0]), float(s2[1])])
        
        self.positions.append(pos)

        self.available = True

    def drawWCS(self, painter: QPainter):     
        pen = QtGui.QPen()
        color = QtGui.QColor()

        color.setRgb(255, 128, 0, 255)
        pen.setColor(color)
        pen.setCapStyle(Qt.RoundCap)
        pen.setWidth(25)
        painter.setPen(pen)
        
        for i in range(len(self.positions)-1):

            pos1 = self.positions[i]*1000
            pos2 = self.positions[i+1]*1000

            painter.drawLine(QLineF(pos1[0], pos1[1], pos2[0], pos2[1]))
