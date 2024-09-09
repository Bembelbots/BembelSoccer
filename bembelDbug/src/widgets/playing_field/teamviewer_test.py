'''Test for Playingfield-widget, main in src/tools/playingfield/widget'''
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
import time

if __name__ == '__main__':
    import sys

    import os
    from os.path import dirname
    sys.path.append(os.path.join(dirname(__file__), "../../"))

from widgets.playing_field.field_objects.hypothese import Hypothese
from widgets.playing_field.field_objects.robot import Robot
from widgets.playing_field.field_objects.obstacle import DetectedRobots
from widgets.playing_field.field_objects.target import ApproachWcsTarget
from widgets.playing_field.field_objects.particles import Particles
from widgets.playing_field.team_viewer_widget import TeamViewerWidget
from utils.vision_result import VisionResult
from widgets.playing_field.config.field_config import SPLFieldConfig
from widgets.playing_field.field_objects.vision_results import VisionResults
from widgets.playing_field.spl_receiver import GCMessage, GCTeamInfo, GCRobotInfo
from utils.constants import Constants
import math



if __name__ == '__main__':
    def generateLinePoints(x, y, length, ystep=0, xstep=0):
        for i in range(0, length):
            vs = VisionResult()
            msg = ""
            msg += "5;"      # vtype
            msg += "0;"      # timestamp 
            msg += "0;"      # ics_x1
            msg += "0;"      # ics_y1
            msg += "0;"      # ics_x2
            msg += "0;"      # ics_y2
            msg += "0;"      # ics_width
            msg += "0;"      # ics_height
            msg += "0;"      # ics_confidence
            msg += str(x) + ";"      # rcs_x1
            msg += str(y) + ";"      # rcs_y1
            msg += str(x) + ";"      # rcs_x2
            msg += str(y) + ";"      # rcs_y2
            msg += "0.0;"      # rcs_alpha
            msg += "0.0;"      # distance
            msg += "1;"      # rcs_confidence
            msg += "1;"      # camera
            msg += "1;"      # extra_int
            msg += "1.0;"      # extra_float
            vs.setFromString(msg)
            yield vs
            y += ystep
            x += xstep

    app = QApplication(sys.argv)
    mainwindow = QMainWindow()
    playing_field = TeamViewerWidget(mainwindow,[],SPLFieldConfig(),True)
    mainwindow.setCentralWidget(playing_field)
    

    #Robots
    # r1 = Robot(1, "richie", (2.0, 1.0), 30.0)
    fieldconfig = SPLFieldConfig()
    robot = Robot("richie", 3,1, fieldconfig)
    anita = Robot("anita", 3,2, fieldconfig)
    anita.setManualPosition(0,0,0)
    moni = Robot("moni", 3,3, fieldconfig)
    moni.setManualPosition(0,0,0)
    headbanger = Robot("headbanger", 3,4, fieldconfig)
    headbanger.setManualPosition(0,0,0)

    message = GCMessage()
    teama = GCTeamInfo()
    teama.teamNumber = 3
    for i in range(Constants.NUM_PLAYERS):
        rinfo = GCRobotInfo()
        if i != 1:
            rinfo.penalty = GCRobotInfo.penalties[2]
            rinfo.penaltyTimeRemaining = "0:23"
        teama.robots.append(rinfo)
    teamb = GCTeamInfo()
    teamb.teamNumber = 4
    message.teams = [teama, teamb]

    anita.handleGCMessage(message)
    moni.handleGCMessage(message)
    headbanger.handleGCMessage(message)
    
    #Hypotheses
    h = Hypothese()
    h.hypos = [
        (2, -1, 0, 1.0),
        (2, -0.5, 0.5, 1.0),
        (2, -1.5, 0, 1.0),
    ]

    h.available = True
    h.visible = True


    #VisionResults

    vss = VisionResults()

    for vs in generateLinePoints(0.5, (0.2 * 9) / 2, 10, ystep=-0.2):
        vss._results.append(vs)

    for vs in generateLinePoints(0.8, (0.2 * 9) / 2 + 0.1, 10, xstep=-0.2):
        vss._results.append(vs)

    for vs in generateLinePoints(2.5, (0.2 * 9) / 2, 10, ystep=-0.2):
        vss._results.append(vs)

    robot.setManualPosition(2.0, 1.0, 0.5)


    vs = VisionResult()
    msg = ""
    msg += "5;"      # vtype
    msg += "0;"      # timestamp 
    msg += "0;"      # ics_x1
    msg += "0;"      # ics_y1
    msg += "0;"      # ics_x2
    msg += "0;"      # ics_y2
    msg += "0;"      # ics_width
    msg += "0;"      # ics_height
    msg += "0;"      # ics_confidence
    msg += str(1) + ";"      # rcs_x1
    msg += str(-1) + ";"      # rcs_y1
    msg += str(1) + ";"      # rcs_x2
    msg += str(1) + ";"      # rcs_y2
    msg += "0.0;"      # rcs_alpha
    msg += "0.0;"      # distance
    msg += "1;"      # rcs_confidence
    msg += "1;"      # camera
    msg += "1;"      # extra_int
    msg += "1.0;"      # extra_float
    vs.setFromString(msg)#
    vss._results.append(vs)

    vs = VisionResult()
    msg = ""
    msg += "6;"      # vtype
    msg += "0;"      # timestamp 
    msg += "0;"      # ics_x1
    msg += "0;"      # ics_y1
    msg += "0;"      # ics_x2
    msg += "0;"      # ics_y2
    msg += "0;"      # ics_width
    msg += "0;"      # ics_height
    msg += "0;"      # ics_confidence
    msg += str(0) + ";"      # rcs_x1
    msg += str(2) + ";"      # rcs_y1
    msg += str(1) + ";"      # rcs_x2
    msg += str(1) + ";"      # rcs_y2
    msg += "0.0;"      # rcs_alpha
    msg += "0.0;"      # distance
    msg += "1;"      # rcs_confidence
    msg += "1;"      # camera
    msg += "1;"      # extra_int
    msg += str(math.pi/2) + ";"      # extra_float
    vs.setFromString(msg)#
    vs.vtype = vs.setType(vs.vtype)
    vss._results.append(vs)


    vs = VisionResult()
    msg = ""
    msg += "7;"      # vtype
    msg += "0;"      # timestamp 
    msg += "0;"      # ics_x1
    msg += "0;"      # ics_y1
    msg += "0;"      # ics_x2
    msg += "0;"      # ics_y2
    msg += "0;"      # ics_width
    msg += "0;"      # ics_height
    msg += "0;"      # ics_confidence
    msg += str(0) + ";"      # rcs_x1
    msg += str(-2) + ";"      # rcs_y1
    msg += str(1) + ";"      # rcs_x2
    msg += str(1) + ";"      # rcs_y2
    msg += "0.0;"      # rcs_alpha
    msg += "0.0;"      # distance
    msg += "1;"      # rcs_confidence
    msg += "1;"      # camera
    msg += "1;"      # extra_int
    msg += str(3*math.pi/2) + ";"      # extra_float
    vs.setFromString(msg)#
    vs.vtype = vs.setType(vs.vtype)
    vss._results.append(vs)

    vs = VisionResult()
    msg = ""
    msg += "8;"      # vtype
    msg += "0;"      # timestamp 
    msg += "0;"      # ics_x1
    msg += "0;"      # ics_y1
    msg += "0;"      # ics_x2
    msg += "0;"      # ics_y2
    msg += "0;"      # ics_width
    msg += "0;"      # ics_height
    msg += "0;"      # ics_confidence
    msg += str(0) + ";"      # rcs_x1
    msg += str(-1) + ";"      # rcs_y1
    msg += str(1) + ";"      # rcs_x2
    msg += str(1) + ";"      # rcs_y2
    msg += "0.0;"      # rcs_alpha
    msg += "0.0;"      # distance
    msg += "1;"      # rcs_confidence
    msg += "1;"      # camera
    msg += "1;"      # extra_int
    msg += str(0) + ";"      # extra_float
    vs.setFromString(msg)#
    vs.vtype = vs.setType(vs.vtype)
    vss._results.append(vs)

    vs = VisionResult()
    msg = ""
    msg += "9;"      # vtype
    msg += "0;"      # timestamp 
    msg += "0;"      # ics_x1
    msg += "0;"      # ics_y1
    msg += "0;"      # ics_x2
    msg += "0;"      # ics_y2
    msg += "0;"      # ics_width
    msg += "0;"      # ics_height
    msg += "0;"      # ics_confidence
    msg += str(3) + ";"      # rcs_x1
    msg += str(0) + ";"      # rcs_y1
    msg += str(1) + ";"      # rcs_x2
    msg += str(1) + ";"      # rcs_y2
    msg += "0.0;"      # rcs_alpha
    msg += "0.0;"      # distance
    msg += "1;"      # rcs_confidence
    msg += "1;"      # camera
    msg += "1;"      # extra_int
    msg += str(math.pi) + ";"      # extra_float
    vs.setFromString(msg)#
    vs.vtype = vs.setType(vs.vtype)
    vss._results.append(vs)

    vs = VisionResult()
    msg = ""
    msg += "10;"      # vtype
    msg += "0;"      # timestamp 
    msg += "0;"      # ics_x1
    msg += "0;"      # ics_y1
    msg += "0;"      # ics_x2
    msg += "0;"      # ics_y2
    msg += "0;"      # ics_width
    msg += "0;"      # ics_height
    msg += "0;"      # ics_confidence
    msg += str(-2) + ";"      # rcs_x1
    msg += str(0) + ";"      # rcs_y1
    msg += str(1) + ";"      # rcs_x2
    msg += str(1) + ";"      # rcs_y2
    msg += "0.0;"      # rcs_alpha
    msg += "0.0;"      # distance
    msg += "1;"      # rcs_confidence
    msg += "1;"      # camera
    msg += "1;"      # extra_int
    msg += str(math.pi) + ";"      # extra_float
    vs.setFromString(msg)#
    vs.vtype = vs.setType(vs.vtype)
    vss._results.append(vs)

    d = DetectedRobots()
    d.obstacles = [(-2.0,0.0), (-1.0,0)]
    d.visible = True
    d.available = True

    target = ApproachWcsTarget()
    target.setFromString("-3,-1@" + str(math.pi/2),None)
    target.visible = True

    particles = Particles()
    particles.particles = [
        (2, 1.2, 0.2),
        (2, 1.4, 0.1),
        (2, 0.5, 0.5),
        (2, 1.5, 0)
    ]
    particles.available = True
    particles.visible = True

    vss.visible = True
    vss.available = True

    robot.addChild(h)
    robot.addChild(vss)
    robot.addChild(d)
    robot.addChild(target)
    robot.addChild(particles)
    playing_field.add_object(robot, -1)
    playing_field.add_object(anita)
    playing_field.add_object(moni)
    playing_field.add_object(headbanger)


    mainwindow.show()
    app.exec_()

    # for i in range(0, 100):
    #     robot.orientation -= 1
    #     time.sleep(0.05)

    sys.exit()
