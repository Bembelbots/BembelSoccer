from PyQt5.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtCore import QThread
from PyQt5.QtCore import pyqtSignal

import sys
import os
import subprocess
from pathlib import Path
from tempfile import gettempdir

from p3elf.reader import ELFBase, ELFReader

from ui.naosyncwidget_ui import Ui_NaoSyncWidget

REPO_ROOT = Path(__file__).parents[5]

class BuildInfo:
    branch = "<font color='red'>No build found!</font>"
    commit = date = author = ""
    changes = "0"
    mtime = 0


    def __init__(self, filename=""):
        self.file = Path(filename)
        self.mtime = 0
        self.update()

    def update(self):
        self.available = False
        self.branch = "<font color='red'>No build found!</font>"
        self.commit = date = author = ""
        self.changes = "0"
        self.mtime = 0

        if self.file.is_file():
            mtime = self.file.stat().st_mtime
            if mtime > self.mtime:
                self.mtime = mtime
                try:
                    elf = ELFReader(self.file)
                    bbversion = elf.get_section(".bbversion").decode("utf-8")
                except:
                    return
                self.available = True
                self.fromString(bbversion)

    def fromString(self, string):
        lines = string.splitlines()
        if len(lines) == 5:
            (self.branch, self.commit, self.date, self.author, self.changes) = lines



class SyncProcess(QThread):
    finished = pyqtSignal()

    def __init__(self, parameters, logfile, callback):
        super().__init__()
        self.parameters = parameters
        self.logfile = logfile
        self.finished.connect(callback)
        self.returncode = 1

    def run(self):
        lastLogFile = open(self.logfile, "w")
        self.process = subprocess.Popen(self.parameters, stdout=lastLogFile)
        self.process.wait()

        self.returncode = self.process.returncode
        self.finished.emit()


class NaoSyncWidget(Ui_NaoSyncWidget, QWidget):

    SYNC_SCRIPT = REPO_ROOT.joinpath("soccer/scripts/sync_nao.sh")
    TERM_WRAPPER = REPO_ROOT.joinpath("software/python_libs/termwrapper.sh")
    logFile = Path(gettempdir()).joinpath("BembelDBug-synclog.txt")

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.ip = None
        self.currentNaoVersion = "v6"

        self.updateTimer = QTimer(self)
        self.updateTimer.timeout.connect(self.updateStatus)
        self.updateTimer.start(2000)

        self.btn_sync_frontend.released.connect(self.syncFrontend)
        self.btn_sync_all.released.connect(self.syncAll)

        self.label_log.linkActivated.connect(self.openLog)
        self.btn_back.released.connect(self.back)
        self.success_widget.hide()

        self.label_widget.setEnabled(False)
        self.button_widget.setEnabled(False)

        self.get_robot_backend_info_process = None
        self.get_robot_frontend_info_process = None

        buildDir = REPO_ROOT.joinpath("soccer/build")
        self.buildInfo = {
            "v5": {
                "backend": BuildInfo(buildDir.joinpath("v5/lib/libjsbackendnaoqi.so")),
                "frontend": BuildInfo(buildDir.joinpath("v5/bin/jsfrontend"))
            },
            "v6": {
                "backend": BuildInfo(buildDir.joinpath("v6/bin/lola-backend")),
                "frontend": BuildInfo(buildDir.joinpath("v6/bin/jsfrontend"))
            }
        }

        self.labels = {
            "backend": [
                self.label_backend_branch,
                self.label_backend_commit,
                self.label_backend_date
            ],
            "frontend": [
                self.label_frontend_branch,
                self.label_frontend_commit,
                self.label_frontend_date
            ]
        }
        
    def __del__(self):
        self.logFile.unlink(True)

    def openLog(self):
        result = subprocess.call([self.TERM_WRAPPER, "-e", "less", "-R", self.logFile])

    def back(self):
        self.label_widget.show()
        self.success_widget.hide()
        self.button_widget.show()
    
    def updateLabels(self):
        for i in ["backend", "frontend"]:
            bi = self.buildInfo[self.currentNaoVersion][i]
            branch, commit, date = self.labels[i]

            branch.setText("branch: " + bi.branch)
            if int(bi.changes) > 0:
                commit.setText("commit " + bi.commit + ", " + bi.changes + " modified")
                date.setText("date " + bi.date + " by " + bi.author)
            else:
                commit.setText("commit " + bi.commit)
                date.setText("date " + bi.date)

    def handleRobotBackendData(self, string):
        bi = BuildInfo()
        if string != None and string != "":
            bi.fromString(string)

        self.label_backend_branch_robot.setText("branch: " + bi.branch)
        self.label_backend_commit_robot.setText("commit " + bi.commit + ", " + bi.changes + " modified")
        self.label_backend_date_robot.setText("built " + bi.date + " by " + bi.author)

    def handleRobotFrontendData(self, string):
        bi = BuildInfo()
        if string != None and string != "":
            bi.fromString(string)

        self.label_frontend_branch_robot.setText("branch: " + bi.branch)
        self.label_frontend_commit_robot.setText("commit " + bi.commit + ", " + bi.changes + " modified")
        self.label_frontend_date_robot.setText("built " + bi.date + " by " + bi.author)

    def updateStatus(self):
        for i in self.buildInfo.values():
            for bi in i.values():
                bi.update()
        self.updateLabels()

        self.label_widget_robot.setEnabled(self.ip is not None)
        version = self.currentNaoVersion
      
        if version is None:
            self.label_widget.setEnabled(False)
            self.button_widget.setEnabled(False)
            return
            
        self.label_widget.setEnabled(True)
        self.button_widget.setEnabled(True)

        b = self.buildInfo[self.currentNaoVersion]["backend"].available
        f = self.buildInfo[self.currentNaoVersion]["frontend"].available
        self.btn_sync_all.setEnabled(b and f)
        self.btn_sync_frontend.setEnabled(f)

    def setBackendNameAndBuildFolder(self, robotname):
        backendName = self.buildInfo[self.currentNaoVersion]["backend"].file.name
        self.label_backend.setText("Backend to sync: ("+ backendName + ")\t")

    def setActiveRobot(self, nao):
        self.setBackendNameAndBuildFolder(nao.name.lower())

        if (nao.config.buildInfoBackend != ""):
            self.handleRobotBackendData(nao.config.buildInfoBackend)
        
        if (nao.config.buildInfoFrontend != ""):
            self.handleRobotFrontendData(nao.config.buildInfoFrontend)
        
        if nao.config.lan_link and nao.config.lan is not None and nao.config.lan != "":        
            self.ip = nao.config.lan
        elif nao.config.wlan is not None and nao.config.wlan != "":
            self.ip = nao.config.wlan
        else:
            self.ip = None

        self.updateStatus()

    def showSuccessPage(self):
        self.label_widget.hide()
        self.button_widget.hide()
        self.success_widget.show()
        self.label_log.setVisible(False)

    def syncFrontend(self):
        self.showSuccessPage()

        if self.ip is None or self.currentNaoVersion is None:
            self.label_success.setText("<font color='red'>No connection to robot!</font>")
            return

        if not self.buildInfo[self.currentNaoVersion]["frontend"].available:
            self.label_success.setText("<font color='red'>No frontend available!</font>")
            return

        parameters = [self.SYNC_SCRIPT, "-f", "-n", self.ip]
        self.label_success.setText("Syncing frontend...")

        self.doSync(parameters)
    
    def syncAll(self):
        self.showSuccessPage()

        if self.ip is None or self.currentNaoVersion is None:
            self.label_success.setText("<font color='red'>No connection to robot!</font>")
            return

        if not self.buildInfo[self.currentNaoVersion]["frontend"].available:
            self.label_success.setText("<font color='red'>No frontend available!</font>")
            return

        if not self.buildInfo[self.currentNaoVersion]["backend"].available:
            self.label_success.setText("<font color='red'>No backend available!</font>")
            return

        parameters = [self.SYNC_SCRIPT, "-a", "-n", self.ip]
        self.label_success.setText("Syncing all...")

        self.doSync(parameters)

    def doSync(self, parameters):
        self.showSuccessPage()

        self.syncProcess = SyncProcess(parameters, self.logFile, self.syncHandler)
        self.syncProcess.start()

    def syncHandler(self):
        self.label_log.setVisible(True)

        if self.syncProcess.returncode == 0:
            self.label_success.setText("<font color='green'>Sync successfull!</font>")
        else:
            self.label_success.setText("<font color='red'>Sync failed. Please review log.</font>")
