from typing import List

import math
import numpy as np
from PyQt5 import QtGui
from PyQt5.QtCore import Qt, QPointF, QLineF, QRect
from PyQt5.QtGui import QColor, QFont, QPainter, QPen

from widgets.playing_field.field_objects.field_object import FieldObject
from widgets.playing_field.team_viewer_drawer_widget import TeamViewerDrawerWidget
from utils.coords import DirectedCoord

class OdoViewer(FieldObject):
    DEFAULT_SIZE = 35

    def onVisibleChanged(self, visible):
        self.mcs_positions.clear()
        self.last_pos = None
        self.last_mcs = None

    def __init__(self):
        super().__init__("Odometry Plot")
        self.last_pos = None
        self.last_mcs = None
        self.mcs_positions = []
        self.visible_by_default = False
        self.BlackboardSymbols = ["BodyInterface.mcs", "BehaviorBlackboard.bot_pos"]
        
    def handleBlackboardSymbols(self, data):
        if data.get("BodyInterface", None) is None:
            #print("BodyInterface: Data of this Blackboard not available !!")
            return
        if data.get("BehaviorBlackboard", None) is None:
            #print("BehaviorBlackboard: Data of this Blackboard not available !!")
            return
        body_blackboard = data["BodyInterface"]
        behaviour_blackboard = data["BehaviorBlackboard"]
        if body_blackboard.get("mcs", None) is None:
            print("mcs is not available")
            return
        if behaviour_blackboard.get("bot_pos", None) is None:
            print("bot_pos is not available")
            return
        mcs_string = body_blackboard["mcs"]
        pos_string = behaviour_blackboard["bot_pos"]

        mcs = DirectedCoord.fromBBString(mcs_string)
        pos = DirectedCoord.fromBBString(pos_string)
        
        if self.last_pos is None:
            self.last_pos = pos
            self.last_mcs = mcs
            return

        # see SinasParticleFilter::moveParticles for the magic

        mcs_delta = mcs.toRCS(self.last_mcs)
        mcs_delta.a = mcs.a-self.last_mcs.a
        
        self.last_pos = self.last_pos.walk(mcs_delta)

        self.mcs_positions.append(self.last_pos)

        self.last_mcs = mcs

        self.available = True

    def drawWCS(self, painter: QPainter):     
        pen = QtGui.QPen()
        color = QtGui.QColor()

        color.setRgb(255, 0, 255, 255)
        pen.setColor(color)
        pen.setCapStyle(Qt.RoundCap)
        pen.setWidth(25)
        painter.setPen(pen)
        
        for i in range(len(self.mcs_positions)-1):

            pos1 = self.mcs_positions[i]
            pos2 = self.mcs_positions[i+1]

            painter.drawLine(QPointF(pos1.x, pos1.y)*1000, QPointF(pos2.x, pos2.y)*1000.0)
        
        # if len(self.mcs_positions) > 1:
        #     #idx = np.round(np.linspace(0, len(self.mcs_positions) - 1, 10)).astype(int)

        #     linelength = 80
        #     arrowhead = 20

        #     #for i in idx:
        #     i=0

        #     pos = self.mcs_positions[i]
        #     lineend = np.array([pos[0]+np.cos(pos[2])*linelength, pos[1]+np.sin(pos[2])*linelength])

        #     arr1 = np.array([lineend[0]+np.cos(pos[2]+3*np.pi/4)*arrowhead, lineend[1]+np.sin(pos[2]+3*np.pi/4)*arrowhead])
        #     arr2 = np.array([lineend[0]+np.cos(pos[2]-3*np.pi/4)*arrowhead, lineend[1]+np.sin(pos[2]-3*np.pi/4)*arrowhead])

        #     painter.drawLine(pos[0], pos[1], lineend[0], lineend[1])
        #     painter.drawLine(arr1[0], arr1[1], lineend[0], lineend[1])
        #     painter.drawLine(arr2[0], arr2[1], lineend[0], lineend[1])
