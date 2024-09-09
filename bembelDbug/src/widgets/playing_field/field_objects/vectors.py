from typing import List
import base64
import struct

import math
from PyQt5 import QtGui
from PyQt5.QtCore import Qt, QLineF
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QFont
from PyQt5.QtGui import QPainter
from utils.parseFrameworkClasses import RobotData
from widgets.playing_field.field_objects.field_object import FieldObject

class Vectors(FieldObject):
    def __init__(self):
        super().__init__()
        self.vectors=[] 

    def setFromString(self,s):
        # 0.00,1.00;2.00,3.00;123,0,12#0.00,1.00;2.00,3.00;123,0,12....
        vecs = s.split("#")

        for vec in vecs:
            data = vec.split(";")
            pos1 = data[0].split(",")
            pos1 = (float(pos1[0]), float(pos1[1]))

            pos2 = data[1].split(",")
            pos2 = (float(pos2[0]),float(pos2[1]))

            color = data[2].split(",")
            color = (int(color[0]), int(color[1]), int(color[2]))

            self.vectors.append(Vector(pos1, pos2, color))
            
    
    def drawWCS(self, painter: QPainter):
        for vector in self.vectors:
            pen = QtGui.QPen()
            color = QtGui.QColor()

            painter.save()

            color.setRgb(vector.r, vector.g, vector.b)
            pen.setColor(color)
            pen.setCapStyle(Qt.RoundCap)
            pen.setWidth(40 * self._scale)
            painter.setPen(pen)
            (x1, y1) = (vector.x1*1000, vector.y1*1000)
            (x2, y2) = (vector.x2*1000, vector.y2*1000)
            painter.drawLine(QLineF(x1,y1,x2,y2))
            
            painter.restore()

class Vector():
    def __init__(self, point1: (float, float) = (0.0, 0.0), 
                       point2: (float, float) = (0.0, 0.0), 
                       color: (int, int, int) = (255, 0, 0)):
        self.x1 = point1[0]
        self.y1 = point1[1]
        self.x2 = point2[0]
        self.y2 = point2[1]
        self.r = color[0]
        self.g = color[1]
        self.b = color[2]

