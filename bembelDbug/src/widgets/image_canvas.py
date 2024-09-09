from PyQt5 import QtGui
from PyQt5.QtCore import Qt, QPointF, QLineF, QRectF

from PyQt5.QtWidgets import (
    QWidget,
)

from PyQt5.QtGui import (
    QImage,
    QPainter,
    QPixmap,
    QColor,
)

from bembelapi.nao import NaoCamImage

import consts


class ImageCanvas(QWidget):
    """
    Displays a nao image.

    Use updateImg to set the image manually.
    Or connect to NaoImage-Event with function handleImg.
    """

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self._img = None
        self._visionResultCanvas = None
        self._visionResults = []

        self.debugRectCounter = 0; # Counter controlling color of rect vision result

    def updateImg(self, img: NaoCamImage):
        """
        Set the image that should be drawn.

        :param img: Should be type bembelapi.nao.camimage to draw the image or 
                    None to display nothing.
        """

        if img is None:
            self._img = None
            return

        if self._img is None:
            self._img = QImage()

        if not self._img.loadFromData(img.get_data()):
            print('... failed to load data.')
            self._img = None

        self.update()

    def paintEvent(self, ev):

        painter = QPainter(self)

        if self._img is None:
            painter.fillRect(self.rect(), QColor('black'))
            return

        painter.drawImage(self.rect(), self._img)

        self.debugRectCounter = 0
        for visionResult in self._visionResults:
           self.drawVisionResult(painter, visionResult)

    def updateVisionResults(self, vrs):
        self._visionResults = vrs
        self.update()

    def icsYToLocal(self, icsy):
        ratio = self.height() / consts.IMAGE_HEIGHT
        return int(ratio * icsy)

    def icsXToLocal(self, icsx):
        ratio = self.width() / consts.IMAGE_WIDTH
        return int(ratio * icsx)

    def icsCoordToLocal(self, icsx, icsy):
        return self.icsXToLocal(icsx), self.icsYToLocal(icsy)

    def drawIcsPoint(self, painter, pen, icsx, icsy):
        painter.setPen(pen)
        sx, sy = self.icsCoordToLocal(icsx, icsy)
        painter.drawPoint(QPointF(sx, sy))

    def drawIcsCircle(self, painter, pen, icsx, icsy, width):
        painter.setPen(pen)
        sx, sy = self.icsCoordToLocal(icsx, icsy)
        painter.drawEllipse(QPointF(sx - (width / 2.0), sy - (width /2.0)), width, width)

    def drawVisionResult(self, painter, visionResult):
        vr = visionResult

        pen = QtGui.QPen()
        c = QtGui.QColor()

        if vr.vtype == "ball":
            c.setRgb(255, 0, 0, 128)
            pen.setColor(c)
            pen.setCapStyle(Qt.RoundCap)
            pen.setWidth(self.icsXToLocal(vr.ics_width)) 
            self.drawIcsPoint(painter, pen, vr.ics_x1, vr.ics_y1)

        elif vr.vtype == "scanpoint":
            c.setRgb(0, 0, 0,255)
            pen.setColor(c)
            pen.setCapStyle(Qt.RoundCap)
            pen.setWidth(1) #self.imageWidth/60)
            self.drawIcsPoint(painter, pen, vr.ics_x1, vr.ics_y1)

        elif vr.vtype == "line":
            c.setRgb(0, 0, 255, 128)
            pen.setColor(c)
            pen.setWidth(self.icsXToLocal(5))
            painter.setPen(pen)
            sx, sy = self.icsCoordToLocal(vr.ics_x1, vr.ics_y1)
            ex, ey = self.icsCoordToLocal(vr.ics_x2, vr.ics_y2)
            painter.drawLine(QLineF(sx,sy,ex,ey))

        elif vr.vtype == "lcrossing" or vr.vtype == "tcrossing" or vr.vtype == "xcrossing" or vr.vtype == "penalty":
            px,py = self.icsCoordToLocal(vr.ics_x1, vr.ics_y1)
            c.setRgb(255, 64, 255, 128)
            if(vr.vtype == "penalty"):
                c.setRgb(255, 193, 0, 193)
            pen.setColor(c)
            pen.setCapStyle(Qt.RoundCap)
            pen.setWidth(25)
            painter.setPen(pen)
            painter.drawPoint(QPointF(px, py))
            
            font = QtGui.QFont("Arial", 16)
            painter.setFont(font)
            t = str(vr.extra_int)
            if vr.vtype == "lcrossing":
                t = "L"
            elif vr.vtype == "tcrossing":
                t = "T"
            elif vr.vtype == "xcrossing":
                t = "X"
            elif vr.vtype == "penalty":
                t = "P"
            c.setRgb(255, 64, 255, 255)
            pen.setColor(c)
            (sx,sy) = (px+self.icsXToLocal(20), py)
            painter.drawText(sx, sy, t)

        elif vr.vtype == "goal":
            px, py = self.icsCoordToLocal(vr.ics_x1, vr.ics_y1)
            w,h = self.icsCoordToLocal(vr.ics_width, vr.ics_height)
            
            c.setRgb(255, 255, 0, 128)
            pen.setColor(c)
            pen.setWidth(10)
            painter.setPen(pen)
            sx,sy = px-w/2, py-h
            w,h   = px-w/2, py-h
            painter.drawRect(QRectF(sx, sy, w, h))

        elif vr.vtype == "circle":
            c.setRgb(0, 255, 255, 128)
            pen.setColor(c)
            pen.setCapStyle(Qt.RoundCap)
            pen.setWidth(2)
            #self.drawIcsPoint(painter, pen, vr.ics_x1, vr.ics_y1)
            self.drawIcsCircle(painter, pen, vr.ics_x1, vr.ics_y1, 20)

        elif vr.vtype == "rect":
            sx, sy = self.icsCoordToLocal(vr.ics_x1, vr.ics_y1)
            w, h   = self.icsCoordToLocal(vr.ics_width, vr.ics_height)
            ex, ey = self.icsCoordToLocal(vr.ics_x2, vr.ics_y2)
            wc     = vr.extra_int

            if (w == 0 or h == 0):
                w = ex + sx
                h = ey + sy

            c.setRgb(128, 128, 128, 128)
            pen.setColor(c)
            pen.setWidth(self.icsXToLocal(4))
            painter.setPen(pen)
            painter.drawRect(QRectF(sx, sy, w, h))
            painter.drawText(sx, sy, str(self.debugRectCounter))
            self.debugRectCounter += 1

        elif vr.vtype == "robot":
            sx, sy = self.icsCoordToLocal(vr.ics_x1, vr.ics_y1)
            w, h = self.icsCoordToLocal(vr.ics_width, vr.ics_height)

            c.setRgb(255, 120, 0, 128)
            pen.setColor(c)
            pen.setWidth(self.icsXToLocal(6))
            painter.setPen(pen)
            painter.drawRect(QRectF(sx, sy, w, h))


    def handleImg(self, img, addr):
        """ Handler for NaoImage-Event. """
        self.updateImg(img)
