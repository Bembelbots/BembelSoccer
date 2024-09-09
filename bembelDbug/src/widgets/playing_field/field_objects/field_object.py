from typing import Optional
from PyQt5 import QtCore, QtGui
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
import math


class FieldObject(QStandardItem):

    def __init__(self, name="", point = (0.0, 0.0), parent=None, team_id=None, robot_id=None):
        super().__init__(name)
        self._scale = 1
        self._field = QRect(0, 0, 0, 0)  # type: Optional[QRect]
        self._position = point
        self._niceness = 0
        self._visible = False
        self._parent = parent

        self.setCheckable(True)
        self.setEditable(False)

        self.robot_id = robot_id
        self.team_id = team_id
        self.BlackboardSymbols = []

        # this is just to trigger the property setter once
        self.visible_by_default = False
        self._available = True
        self.available = False
        self.visible_by_default = True        

    @property
    def parent(self):
        return self._parent

    @property
    def position(self):
        return self._position

    @property
    def niceness(self):
        return self._niceness
    
    def onVisibleChanged(self, visible):
        pass

    @niceness.setter
    def niceness(self, niceness: int):
        self._niceness = niceness
    
    @property
    def available(self):
        return self._available

    @available.setter
    def available(self, available: bool):
        if(self._available == available):
            return
        self._available = available
        self.setEnabled(available)
        self.visible = self.visible_by_default

    @property
    def visible(self):
        return self._visible

    @visible.setter
    def visible(self, visible: bool):
        self._visible = visible
        if(visible):
            self.setCheckState(Qt.Checked)
        else:
            self.setCheckState(Qt.Unchecked)

    @property
    def scale(self):
        return self._scale

    @scale.setter
    def scale(self, val: float):
        self._scale = val

    def __lt__(self, other):
        return self._niceness > other.niceness

    def onCheckedChanged(self):
        self._visible = (self.checkState() == Qt.Checked)
        self.onVisibleChanged(self._visible)

    def addChild(self, child):
        child._parent = self
        self.appendRow(child)
    
    def handleGCMessage(self, gcmessage):
        pass

    def handleSPLMessage(self, message):

        

        if self.team_id is None or message.team_num != self.team_id or message.player_num != self.robot_id:
            return

        for i in range(self.rowCount()):
            item = self.child(i)
            item.setFromSPLMessage(message)

        self.setFromSPLMessage(message)

    def drawWCS(self, painter: QPainter):
        pass

    def drawWCSText(self, drawer):
        pass
        
    def handleDrawWCS(self, painter):
        
        if not self.available or not self.visible:
            return
        
        for i in range(self.rowCount()):
            item = self.child(i)
            item.handleDrawWCS(painter)

        self.drawWCS(painter)

    def handleDrawWCSText(self, painter):
        if not self.available or not self.visible:
            return
        
        for i in range(self.rowCount()):
            item = self.child(i)
            item.handleDrawWCSText(painter)

        self.drawWCSText(painter)

    def setFromSPLMessage(self,message):
        pass

    def setFromString(self,message, blackboardsymbol):
        pass

    def registerBlackboardSymbols(self, connection):

        for i in range(self.rowCount()):
            item = self.child(i)
            item.registerBlackboardSymbols(connection)

        for blackboard_symbol in self.BlackboardSymbols:
            blackboard_name, symbol_name = blackboard_symbol.split(".")
            connection.start_receiving_symbol(blackboard_name, symbol_name)

    def unregisterBlackboardSymbols(self, connection):

        for i in range(self.rowCount()):
            item = self.child(i)
            item.unregisterBlackboardSymbols(connection)

        for blackboard_symbol in self.BlackboardSymbols:
            blackboard_name, symbol_name = blackboard_symbol.split(".")
            connection.stop_receiving_symbol(blackboard_name, symbol_name)

    def handleBlackboardSymbols(self, data):
        
        for i in range(self.rowCount()):
            item = self.child(i)
            item.handleBlackboardSymbols(data)

        for blackboard_symbol in self.BlackboardSymbols:
            blackboard_name, symbol_name = blackboard_symbol.split(".")
            if data.get(blackboard_name, None) is None:
                #print(self.BlackboardName + ": Data of this Blackboard not available !!")
                self.available = False
                return
            blackboard = data[blackboard_name]
            if blackboard.get(symbol_name, None) is None:
                #print(self.SymbolName + ": Data of this Symbol not available !!")
                self.available = False
                return
            datastring = blackboard[symbol_name]
            self.setFromString(datastring, blackboard_symbol)

    def _transform_from_rcw_to_wcs(self, point_rcs: (float, float)) -> (float, float):
        x, y = point_rcs

        robot = self.parent

        if self.parent is None:
            print("No parent set! Should be robot.")
            return
        if robot.gtposition is not None:
            (robot_x, robot_y) = robot.gtposition
            robot_alpha = -robot.gtorientation
        else:
            (robot_x, robot_y) = robot.position
            robot_alpha = -robot.orientation

        robot_x, robot_y = (robot_x, robot_y)

        
        

        newX = robot_x + (math.cos(robot_alpha) * x) + (math.sin(robot_alpha) * y)
        newY = robot_y - (math.sin(robot_alpha) * x) + (math.cos(robot_alpha) * y)

        return ((newX, newY))