import numpy as np
from typing import List, Optional
import heapq
import time
from PyQt5 import QtCore, QtSvg, QtGui
from PyQt5.QtCore import QSize, QRect, QPointF, Qt, QTimer
from PyQt5.QtGui import QPainter, QColor, QPen, QPalette, QFont
from PyQt5.QtWidgets import QMainWindow, QWidget
from widgets.playing_field.field_objects.field_object import FieldObject

def npMatrixToQTransform(np_matrix):
    m = np_matrix.getT().tolist()
    return QtGui.QTransform(
        m[0][0], m[0][1], m[0][2],
        m[1][0], m[1][1], m[1][2],
        m[2][0], m[2][1], m[2][2]
    )

class TeamViewerDrawerWidget(QWidget):

    world_transform = np.matrix(np.identity(3), copy=False)

    default_size = QSize(640, 480)

    field_objects = []

    def setupUi(self):
        self.field_renderer = QtSvg.QSvgRenderer(self.field_config.svg_name, self)
        self.setMinimumSize(self.default_size)
        self.resize(self.default_size)
        self.scale = 1
        self.v_padding = 0
        self._man_mirrored = False
        self._gc_mirrored = False
        self.h_padding = 0
        self.redraw_field = True
        self.field = QtCore.QRectF(0, 0, self.width(), self.height())

    def __init__(self, parent, fieldconfig, debug):
        super().__init__(parent)

        self.debug = debug

        self.field_config = fieldconfig

        self.setupUi()

    # manually mirrored by checkbox
    @property
    def man_mirrored(self):
        return self._man_mirrored
    
    @man_mirrored.setter
    def man_mirrored(self, value):
        self._man_mirrored = value
        self.update
 
    # auto mirrored by GC state
    @property
    def gc_mirrored(self):
        return self._gc_mirrored
    
    @gc_mirrored.setter
    def gc_mirrored(self, value):
        self._gc_mirrored = value
        self.update

    def is_mirrored(self):
        # mirror if either GameController says so or "Mirror" checkbox is checked
        # but not if both are true
        return self.man_mirrored ^ self.gc_mirrored
        
    @staticmethod
    def keep_aspect_ratio(w1, h1, w2, h2):
        ratio_width = w1 / w2
        ratio_height = h1 / h2

        scale = ratio_height if ratio_height < ratio_width else ratio_width

        width = w2 * scale
        height = h2 * scale

        return width, height, scale

    def drawTextAtPoint(self, text, posX, posY, offset, fontSize=160, color=(0,0,0)):
        self.painter.save()
        self.painter.setFont(QFont("Monospace", fontSize))
        self.painter.setPen(QPen(QColor(*color), 20, Qt.SolidLine))

        if self.is_mirrored():
            self.painter.translate(-posX, posY)
        else:
            self.painter.translate(posX, -posY)

        text_width = self.painter.fontMetrics().width(text)
        text_position = QPointF(-text_width / 2 + offset[0], fontSize / 2 + offset[1])
        
        self.painter.drawText(text_position, text)
        self.painter.restore()

    def paintEvent(self, event):
        super().paintEvent(event)

        field_size = self.field_renderer.defaultSize()
        width, height, _ = self.keep_aspect_ratio(self.width(), self.height(),
                                                  field_size.width(), field_size.height())

        self.v_padding = (self.height() - height)
        self.h_padding = (self.width() - width)

        self.field = QtCore.QRectF(self.h_padding / 2, self.v_padding / 2, width, height)

        self.onWorldTransform()

        #### Set up the painter object
        self.painter = QPainter()
        self.painter.begin(self)

        self.painter.setRenderHint(QPainter.TextAntialiasing, True)
        self.painter.setRenderHint(QPainter.Antialiasing, True)

        painter_transform = npMatrixToQTransform(self.world_transform)
        textpainter_transform = npMatrixToQTransform(self.text_transform)
        
        #### Actually start painting stuff
        self.field_renderer.render(self.painter, self.field)

        self.painter.setTransform(painter_transform)

        # F = 1000
        # self.painter.setPen(QPen(QColor(255,0,0),0.2*F))
        # self.painter.drawPoint(0,0)

        # self.painter.setPen(QPen(QColor(255,0,255),0.2*F))
        # self.painter.drawPoint(4.5*F,0)

        # self.painter.setPen(QPen(QColor(255,255,0),200))
        # self.painter.drawPoint(2000,1000)

        # self.painter.setPen(QPen(QColor(0,0,255),0.2*F))
        # self.painter.drawPoint(0,3*F)

        # self.painter.setPen(QPen(QColor(0,255,0),0.2*F))
        # self.painter.drawPoint(0,-3*F)

        for child in self.field_objects:
            self.painter.save()
            child.handleDrawWCS(self.painter)
            self.painter.restore()            

        self.painter.setTransform(textpainter_transform)
        # self.drawTextAtPoint("test", 2000, 1000, 100)

        for child in self.field_objects:
            self.painter.save()
            child.handleDrawWCSText(self)
            self.painter.restore()            


        self.painter.end()

    def onWorldTransform(self):
        field_size = self.field_renderer.defaultSize()

        scale_x = self.field.width() / (field_size.width())
        scale_y = self.field.height() / (field_size.height())
                
        self.text_transform = np.matrix([
                [scale_x, 0, self.width() / 2],
                [0, scale_y, self.height() / 2],
                [0, 0, 1]
            ])

        if self.is_mirrored():
            self.world_transform = np.matrix([
                [-scale_x, 0, self.width() / 2],
                [0, scale_y, self.height() / 2],
                [0, 0, 1]
            ])
        else:
            self.world_transform = np.matrix([
                [scale_x, 0, self.width() / 2],
                [0, -scale_y, self.height() / 2],
                [0, 0, 1]
            ])


    def resizeEvent(self, event):
        return super().resizeEvent(event)

    def add_object(self, obj: FieldObject, niceness=0):
        obj.niceness = niceness
        heapq.heappush(self.field_objects, obj)
        self.update()

    def add_objects(self, objects: List[FieldObject], niceness=0):
        for obj in objects:
            obj.niceness = niceness
            heapq.heappush(self.field_objects, obj)
        self.update()

    def remove_object(self, obj):
        self.field_objects.remove(obj)

    def shutdown(self):
        del self.field_objects[:]
        print("Drawer has shut down")
