from PyQt5 import QtCore, QtSvg, QtGui
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *

from widgets.playing_field.team_viewer_drawer_widget import TeamViewerDrawerWidget
from widgets.playing_field.field_objects.field_object import FieldObject
from widgets.playing_field.field_objects.robot import Robot
from widgets.playing_field.field_objects.ball import Ball
from widgets.playing_field.spl_receiver import *
from utils.constants import Constants

class TeamViewerWidget(QWidget):

    objects = []

    def checkedChanged(self, item):
        item.onCheckedChanged()

    def setupUi(self):
        self.setWindowTitle("TeamViewer")
        self.mainlayout = QHBoxLayout(self)
        self.splitter = QSplitter(Qt.Horizontal)

        self.sidebar = QWidget(self)
        self.sidebar.setLayout(QVBoxLayout())


        self.checkablelist = QTreeView(self)
        self.checkablelistmodel = QStandardItemModel()
        self.checkablelist.setModel(self.checkablelistmodel)
        self.checkablelist.setSizePolicy(QSizePolicy.Minimum,QSizePolicy.Minimum)
        self.checkablelist.setMinimumWidth(200)

        self.teamviewerdrawer.setSizePolicy(QSizePolicy.Expanding,QSizePolicy.Expanding)

        self.sidebar.layout().addWidget(self.checkablelist)

        self.mirror_checkbox = QCheckBox(self)
        self.mirror_checkbox.setText("Mirror")
        self.mirror_checkbox.stateChanged.connect(self.handleMirrorClicked)
        self.sidebar.layout().addWidget(self.mirror_checkbox)

        self.splitter.addWidget(self.teamviewerdrawer)    
        self.splitter.addWidget(self.sidebar)

        self.mainlayout.addWidget(self.splitter)

        self.mousePosWcs = None

        self.checkablelistmodel.itemChanged.connect(self.checkedChanged)

        self.setLayout(self.mainlayout)
    
    def mousePressEvent(self, QMouseEvent):
        offset_x = self.teamviewerdrawer.world_transform[0,2]
        scale_x = self.teamviewerdrawer.world_transform[0,0]
        offset_y = self.teamviewerdrawer.world_transform[1,2]
        scale_y = self.teamviewerdrawer.world_transform[1,1]
        x,y= QMouseEvent.pos().x(), QMouseEvent.pos().y()
        self.mousePosWcs = (((x-offset_x)/(scale_x))/1000, ((y-offset_y)/(scale_y))/1000)

    def handleMirrorClicked(self, state):
        self.teamviewerdrawer.man_mirrored = (state == Qt.Checked)

    def addTeamReceiver(self, team):
        try:
            spl_receiver = SPLReceiver(10000+team)
            spl_receiver.message.connect(self.onsplmessage)
            spl_receiver.start()
            self.spl_receivers[team] = spl_receiver
            print("Started SPL receiver for team " + str(team))
        except:
            print("Failed to start SPL receiver for team " + str(team))

    def __init__(self, parent, spl_teamids, fieldconfig, listen_GC=True):
        super().__init__(parent)
        self.teamviewerdrawer = TeamViewerDrawerWidget(self, fieldconfig, True)
        self.setupUi()
        self.gc_receiver = None
        self.spl_receivers = {}

        if(spl_teamids != None):
            for team in spl_teamids:
                self.addTeamReceiver(team)
            
        if(listen_GC):
            self.gc_receiver = GCReceiver()
            self.gc_receiver.message.connect(self.ongcmessage)
            self.gc_receiver.start()

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update)
        self.timer.start(200)   
    
    def add_object(self, obj: FieldObject, niceness=0):
        self.teamviewerdrawer.add_object(obj, niceness)
        self.checkablelistmodel.appendRow(obj)
        self.objects.append(obj)

    def remove_object(self, obj):
        self.teamviewerdrawer.remove_object(obj)
        self.checkablelistmodel.removeRow(obj.index().row(),1)
        if not obj in self.objects:
            self.objects.remove(obj)

    def onsplmessage(self, splmessage):
        for obj in self.objects:
            obj.handleSPLMessage(splmessage)
        
    def ongcmessage(self, gcmessage):
        teams = (gcmessage.teams[0].teamNumber, gcmessage.teams[1].teamNumber)

        # set mirrored state according to GC info
        home = (teams[0] == Constants.TEAM_ID)
        away = (teams[1] == Constants.TEAM_ID)

        # only mirror or start listener, if GC packet contains our team teamNumber
        if home or away:
            first = gcmessage.half == "First"
            self.teamviewerdrawer.gc_mirrored = (home and not(first)) or (away and first)
            # start teamcomm listener if not running yet
            for t in teams:
                if not self.spl_receivers.get(t):
                    self.addTeamReceiver(t)

        for obj in self.objects:
            obj.handleGCMessage(gcmessage)

    def shutdown(self):
        self.timer.stop()
        self.teamviewerdrawer.shutdown()
