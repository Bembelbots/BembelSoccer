import base64
import struct

import math

from typing import Dict
from typing import Optional

from PyQt5 import QtWidgets
from bembelapi.config import NaoEvent
from bembelapi.modules.spl import SPLStandardMessage

from debugger import BembelDbugFactory
from tool_management.toolsregistry import ConnectionTool
from utils.vision_result import VisionResult
from widgets.playing_field.field_objects.hypothese import Hypothese
from widgets.playing_field.field_objects.particles import Particles
from widgets.playing_field.field_objects.obstacle import DetectedRobots
from widgets.playing_field.field_objects.robot import Robot
from widgets.playing_field.field_objects.teamball import TeamBall
from widgets.playing_field.field_objects.ball import Ball
from widgets.playing_field.field_objects.odo_viewer import OdoViewer
from widgets.playing_field.field_objects.position_plot import PositionPlot
from widgets.playing_field.field_objects.vision_results import VisionResults
from widgets.playing_field.config.field_config import FieldConfig
from widgets.playing_field.field_objects.target import ReactiveWalkTarget, ApproachWcsTarget

from widgets.playing_field.team_viewer_widget import TeamViewerWidget

'''
HOWTO ADD AN OBJECT TO THE PLAYINGFIELDTOOL:
1. Create a class in widgets/playing_field/field_objects that parses and draws your data send by framework:
    your class should habe a setFromString and a draw method.
    The default FieldObject implementation needs you to set self.BackboardName and self.SymbolName.
    Alternatively, you can override registerBlackboardSymbols, unregisterBlackboardSymbols and handleBlackboardSymbols yourself.
    This can be useful if you need multiple data points.

    If your Data is in rcs, you can set the robot as parent (see e.g. VisionResults) and use the parent in the class for the
    robots position.
    
2. add your class  to the fieldObjects
'''

class DebugObject():
    def __init__(self,blackboard_name, symbol_name, value, niceness= 0):
        self.blackboard_name = blackboard_name
        self.symbol_name = symbol_name
        self.value = value
        self.niceness = niceness


class DebugFieldObject():
    def __init__(self, value, niceness= 0):
        self.value = value
        self.niceness = niceness

class PlayingFieldWidget(ConnectionTool):
    name = "playing_field"
    button_text = "Playing Field"

    def __init__(self):
        ConnectionTool.__init__(self, self.name, self.button_text)
        
        self.field = None  

        self.fieldObjects = {}
        self.settings = {}
        self.settings.update({"robot_id":DebugObject("RobotSettings", "id",0)})
        self.settings.update({"fieldSize":DebugObject("RobotSettings", "fieldSize", 1)})
   
        self.events = BembelDbugFactory.get()
        self.events.add_listener(NaoEvent.NAO_SYMBOL_VALUES, self.handle_symbol_values)

        self.gt_clickpos1 = None
        self.gt_clickpos2 = None

    def setupUi(self, widget,data):
        fieldId = 1
        if "fieldSize" not in self.settings:
            print("Cannot access RobotSettings Blackboard, choosing SPL field")    
        else:
            fieldId = self.settings["fieldSize"].value

        fieldConfig = FieldConfig.fromID(fieldId)

        # we dont need spl messages, pass None as teams                   
        self.field = TeamViewerWidget(parent=self, spl_teamids=None, fieldconfig=fieldConfig)

        self.v_layout = QtWidgets.QVBoxLayout(self)

        for num in range(1,8):
            robot_obj = Robot("BB Roboter " + str(num), 3, num, fieldConfig)     
            robot_obj.addChild(Ball("seen ball", 3, num))
            if(self.settings["robot_id"].value == num-1):
                robot_obj.addChild(TeamBall())
                robot_obj.addChild(VisionResults())
                robot_obj.addChild(Particles())
                robot_obj.addChild(DetectedRobots())
                robot_obj.addChild(OdoViewer())
                robot_obj.addChild(PositionPlot())
                robot_obj.addChild(ReactiveWalkTarget())
                robot_obj.addChild(ApproachWcsTarget())
            self.fieldObjects.update({"robot_"+ str(num) : DebugFieldObject(robot_obj, -1)})
        
        for obj in self.fieldObjects.values():
            obj.value.registerBlackboardSymbols(self.connection)

        for obj in self.fieldObjects:
            self.field.add_object(self.fieldObjects[obj].value, self.fieldObjects[obj].niceness)
        
        self.v_layout.addWidget(self.field)
        self.setLayout(self.v_layout)

    def setup(self):
        super().setup()
        for obj in self.settings.values():
            self.connection.start_receiving_symbol(obj.blackboard_name, obj.symbol_name)

    def shutdown(self):
        super().shutdown()
        self.events.remove_listener(NaoEvent.NAO_SYMBOL_VALUES, self.handle_symbol_values)

        for obj in self.fieldObjects.values():
            obj.value.unregisterBlackboardSymbols(self.connection)
            del obj

        for obj in self.settings.values():
            self.connection.stop_receiving_symbol(obj.blackboard_name, obj.symbol_name)

        self.field.shutdown()
        del self.field

    def handle_symbol_values(self, data, _):
        
        # wait until loading the gui until we know the field size
        if self.field == None:
            self.loadSettings(data)
            self.setupUi(self,data)

        self.update_objects_from_framworkdata(data)
        if self.field.mousePosWcs is not None:
            if self.gt_clickpos1 is None:
                self.gt_clickpos1 = self.field.mousePosWcs
                print("got one position, wait for second pos for orientation")
            else:
                self.gt_clickpos2 = self.field.mousePosWcs
                self.setRobotGT(self.gt_clickpos1,self.gt_clickpos2)
                self.gt_clickpos1 = None
                self.gt_clickpos2 = None
        self.field.mousePosWcs = None
        self.field.update()

    def setRobotGT(self, pos1, pos2):
        for key in self.fieldObjects:
            if "robot" in key:
                obj = self.fieldObjects[key].value
                obj.gtposition = pos1
                obj.gtorientation = math.atan2(pos2[1]-pos1[1], pos2[0]-pos1[0])


    def loadSettings(self,data):
        for obj in self.settings.values():
            blackboard = data.get(obj.blackboard_name, None)
            if blackboard is None:
                print(obj.blackboard_name + ": Data of this Blackboard not available !!")
                continue
            for symbol in blackboard.keys():
                if symbol == obj.symbol_name:
                    #TODO: What if not int ?? :/
                    obj.value = int(blackboard[symbol])

    def update_objects_from_framworkdata(self, data):        
        for obj in self.fieldObjects.values():
            obj.value.handleBlackboardSymbols(data)
