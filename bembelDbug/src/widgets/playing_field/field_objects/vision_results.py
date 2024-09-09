from typing import List

import base64
import struct

import math
from PyQt5 import QtGui
from PyQt5.QtCore import Qt, QPointF, QRectF, QLineF
from PyQt5.QtGui import QColor
from PyQt5.QtGui import QFont
from PyQt5.QtGui import QPainter

from utils.vision_result import VisionResult
from widgets.playing_field.field_objects.field_object import FieldObject
from widgets.playing_field.team_viewer_drawer_widget import TeamViewerDrawerWidget


class VisionResults(FieldObject):
    def __init__(self):
        super().__init__("VisionResults")
        self._results = []  # type: List(VisionResult)
        self.BlackboardSymbols = ["VisionData._visionResults"]

    @property
    def results(self):
        return self._results

    @results.setter
    def results(self, results: List[VisionResult]):
        self._results = results

    def setFromString(self,s, _):
        elementSize = 76
        results = []
        visionResult = base64.b64decode(s)
        elements = struct.unpack_from('i', visionResult)[0]
        visionResult = visionResult[4:]
        for i in range(elements):
            element = visionResult[:elementSize]
            visionResult = visionResult[elementSize:]
            vr = VisionResult(element)
            results.append(vr)

        self._results = results

        self.available = True

    def drawWCS(self, painter: QPainter):

        if self.parent is None:
            print("No parent for vision result set! Should be robot.")
            return

        for result in self._results:
            painter.save()
            self._drawVisionResult(painter, result)
            painter.restore()


    # move to other position in code
    def lineDestination(self, px, py, a, l):
        pxd = l * math.cos(a)
        pyd = l * math.sin(a)

        return (px + pxd, py + pyd)

    def _drawVisionResult(self, painter: QPainter, vr: VisionResult):
        pen = QtGui.QPen()
        color = QtGui.QColor()
        
        if vr.vtype in ["line",5]: #line
            color.setRgb(0, 0, 255, 255)
            pen.setColor(color)
            pen.setWidth(40)
            painter.setPen(pen)
            sx, sy = self._transform_from_rcw_to_wcs((vr.rcs_x1, vr.rcs_y1))
            ex, ey = self._transform_from_rcw_to_wcs((vr.rcs_x2, vr.rcs_y2))
            painter.drawLine(QPointF(sx, sy)*1000, QPointF(ex, ey)*1000)

        elif vr.vtype in ["circle"]:
            (px, py) = self._transform_from_rcw_to_wcs((vr.rcs_x1, vr.rcs_y1))
            
            px *= 1000
            py *= 1000

            color.setRgb(0, 0, 255, 255)
            pen.setColor(color)
            pen.setCapStyle(Qt.RoundCap)
            pen.setWidth(20)
            painter.setPen(pen)

            # circle:
            robot_size = 300
            half_size = robot_size / 2
            startAngle = 30 * 16
            spanAngle = 360 * 16

            robot = QRectF(px - half_size, py- half_size, robot_size, robot_size)

            painter.drawArc(robot, startAngle, spanAngle)

            # lines:
            (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float, 0.2))
            pxt *= 1000
            pyt *= 1000
            painter.drawLine(QLineF(px, py, pxt, pyt))

            (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float - math.pi, 0.2))
            pxt *= 1000
            pyt *= 1000
            painter.drawLine(QLineF(px, py, pxt, pyt))

        elif vr.vtype == "penalty":
            (px, py) = self._transform_from_rcw_to_wcs((vr.rcs_x1, vr.rcs_y1))

            px *= 1000
            py *= 1000

            color.setRgb(255, 255, 0, 255)
            pen.setColor(color)
            pen.setWidth(50)
            painter.setPen(pen)

            (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float, 0.05))
            pxt *= 1000
            pyt *= 1000
            painter.drawLine(QLineF(px, py, pxt, pyt))

            (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float + 0.5 * math.pi, 0.05))
            pxt *= 1000
            pyt *= 1000
            painter.drawLine(QLineF(px, py, pxt, pyt))

            (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float - 0.5 * math.pi, 0.05))
            pxt *= 1000
            pyt *= 1000
            painter.drawLine(QLineF(px, py, pxt, pyt))

            (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float - math.pi, 0.05))
            pxt *= 1000
            pyt *= 1000
            painter.drawLine(QLineF(px, py, pxt, pyt))

        elif vr.vtype in ["lcrossing", "tcrossing", "xcrossing", 6,7,8]:#["lcrossing", "tcrossing", "xcrossing"] 
            (px, py) = self._transform_from_rcw_to_wcs((vr.rcs_x1, vr.rcs_y1))

            px *= 1000
            py *= 1000

            color2 = QtGui.QColor()

            # set color for orientation line, color2 for extra lines
            color.setRgb(0, 0, 255,255)
            if vr.vtype == "lcrossing":
                color.setRgb(0, 255, 255, 255)
                color2.setRgb(0, 255, 255, 180)
            elif vr.vtype == "tcrossing":
                color.setRgb(255, 0, 255, 255)
                color2.setRgb(255, 0, 255, 200)
            elif vr.vtype == "xcrossing":
                color.setRgb(255, 128, 0, 255)
                color2.setRgb(255, 128, 0, 200)

            pen.setColor(color)
            pen.setCapStyle(Qt.RoundCap)
            pen.setWidth(100)
            painter.setPen(pen)
            painter.drawPoint(QPointF(px, py))
            pen.setWidth(50)
            painter.setPen(pen)

            #draw orientation
            (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float, 0.2))
            pxt *= 1000
            pyt *= 1000
            painter.drawLine(QLineF(px, py, pxt, pyt))

            #draw extra lines
            pen.setColor(color2)
            painter.setPen(pen)

            if vr.vtype in ["lcrossing", "tcrossing", "xcrossing"]:
                #       |
                #       |
                #   + + x
                (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float + 0.5 * math.pi, 0.2))
                pxt *= 1000
                pyt *= 1000
                painter.drawLine(QLineF(px, py, pxt, pyt))

            if vr.vtype in ["tcrossing", "xcrossing"]:
                #       |
                #       |
                #   ----x + +    
                (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float - 0.5 * math.pi, 0.2))
                pxt *= 1000
                pyt *= 1000
                painter.drawLine(QLineF(px, py, pxt, pyt))

            if vr.vtype == "xcrossing":
                #       |
                #   ----x----
                #       +    
                (pxt, pyt) = self._transform_from_rcw_to_wcs(self.lineDestination(vr.rcs_x1, vr.rcs_y1, vr.extra_float - math.pi, 0.2))
                pxt *= 1000
                pyt *= 1000
                painter.drawLine(QLineF(px, py, pxt, pyt))


                
    def drawWCSText(self, drawer):

        for vr in self._results:
            if vr.vtype in ["lcrossing", "tcrossing", "xcrossing", "penalty",6,7,8]:
                t = "None"
                #text
                if vr.vtype == "lcrossing":
                    t = "L"
                elif vr.vtype == "tcrossing":
                    t = "T"
                elif vr.vtype == "xcrossing":
                    t = "X"
                elif vr.vtype == "penalty":
                    t = "P"

                (px, py) = self._transform_from_rcw_to_wcs((vr.rcs_x1, vr.rcs_y1))

                px *= 1000
                py *= 1000

                # drawer.drawTextAtPoint(t, px, py, (0,0))
            elif vr.vtype in ["circle"]:
                (px, py) = self._transform_from_rcw_to_wcs((vr.rcs_x1, vr.rcs_y1))
                px *= 1000
                py *= 1000
                # drawer.drawTextAtPoint("center", px, py, (0,0))

