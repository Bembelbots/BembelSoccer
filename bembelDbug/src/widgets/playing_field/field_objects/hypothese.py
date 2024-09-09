import math

from PyQt5.QtCore import QPointF
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QPainter
from PyQt5.QtGui import QPen
from PyQt5.QtCore import Qt

from widgets.playing_field.field_objects.field_object import FieldObject


class Hypothese(FieldObject):

    def __init__(self):
        super().__init__("Hypotheses")
        self._hypos = []
        self.BlackboardSymbols = ["PoseData.Hypotheses"]

    @property
    def hypos(self):
        return self._hypos

    @hypos.setter
    def hypos(self, hypos):
        self._hypos = hypos

    def setFromString(self,s,_):
        self._hypos = []
        if len(s) == 0:
            return

        hyp = s.split(";")
        hypos = []

        for h in hyp:
            (pos, angle) = h.split("@")
            (x, y) = pos.split(",")
            angle = float(angle)
            x = float(x)
            y = float(y)
            hypos.append([x, y, angle, 1.0])
            
        self._hypos = hypos
        self.available = True

    def drawWCS(self, painter: QPainter):
        for (rx, ry, ra, rc) in self._hypos:
            painter.save()
            c = QColor()
            c.setRgb(250, 100, 150)

            pen = QPen()
            pen.setColor(c)

            # draw point
            pen.setWidth(120)
            pen.setCapStyle(Qt.RoundCap)
            painter.setPen(pen)
            painter.drawPoint(QPointF(rx, ry)*1000)

            # rotate
            pen.setWidth(20)
            painter.setPen(pen)
            painter.translate(QPointF(rx, ry)*1000)
            painter.rotate(math.degrees(ra))

            painter.drawLine(0, 0, 200, 0)
            painter.restore()
