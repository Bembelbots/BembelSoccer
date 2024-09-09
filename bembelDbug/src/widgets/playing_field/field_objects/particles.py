import math

from PyQt5.QtCore import QPointF
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QPainter
from PyQt5.QtGui import QPen

from widgets.playing_field.field_objects.field_object import FieldObject


class Particles(FieldObject):

    def __init__(self):
        super().__init__("Particles")
        self._particles = []
        self.BlackboardSymbols = ["PoseData.Particles"]

    @property
    def particles(self):
        return self._particles

    @particles.setter
    def particles(self, particles):
        self._particles = particles

    def setFromString(self,s, _):
        particles = []
        if len(s) == 0:
            return

        part = s.split(";")
        

        for h in part:
            (pos, angle) = h.split("@")
            (x, y) = pos.split(",")
            angle = float(angle)
            x = float(x)
            y = float(y)
            particles.append([x, y, angle])
            
        self._particles = particles
        self.available = True

    def drawWCS(self, painter: QPainter):
        for (rx, ry, ra) in self._particles:
            painter.save()
            c = QColor()
            c.setRgb(0, 0, 0)

            pen = QPen()
            pen.setColor(c)

            # draw point
            pen.setWidth(50)
            painter.setPen(pen)
            painter.drawPoint(QPointF(rx, ry)*1000)

            # rotate
            pen.setWidth(10)
            painter.setPen(pen)
            painter.translate(QPointF(rx, ry)*1000)
            painter.rotate(math.degrees(ra))

            painter.drawLine(0, 0, 150, 0)
            painter.restore()
