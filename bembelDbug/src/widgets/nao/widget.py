from typing import Optional
import time

from PyQt5.QtWidgets import QFrame, QAction
from PyQt5.QtCore import Qt
from PyQt5.QtGui import *

from bembelapi.config import NaoMonitorAction
from bembelapi.nao import NaoInfo
from bembelapi.naoconnection import NaoConnection
from config import NaoStatusColors
from debugger import BembelDbugFactory
from ui.naolist.nao_ui import Ui_NaoWidget
from PyQt5.QtCore import pyqtSignal


class NaoWidget(Ui_NaoWidget, QFrame):

    sync_frontend_signal = pyqtSignal(NaoInfo)
    sync_all_signal = pyqtSignal(NaoInfo)
    selected_nao = pyqtSignal(NaoInfo, bool)

    def __init__(self, parent: 'NaoListWidget', nao: NaoInfo):
        super().__init__()
        self._parent = parent
        self._nao = nao
        self._connection = None  # type: Optional[NaoConnection]
        self._attemptConnection = False
        self.setupUi(self)
        self.setContextMenuPolicy(Qt.ActionsContextMenu)

        self.ssh_action_lan = QAction("ssh (eth)", self)
        self.ssh_action_lan.triggered.connect(lambda: self.nao.do_ssh("eth"))
        self.addAction(self.ssh_action_lan)

        self.ssh_action_wlan = QAction("ssh (wlan)", self)
        self.ssh_action_wlan.triggered.connect(lambda: self.nao.do_ssh("wlan"))
        self.addAction(self.ssh_action_wlan)

        self.sync_frontend_action = QAction("sync frontend", self)
        self.sync_frontend_action.triggered.connect(lambda: self.sync_frontend_signal.emit(self.nao))
        self.addAction(self.sync_frontend_action)

        self.sync_all_action = QAction("sync all", self)
        self.sync_all_action.triggered.connect(lambda: self.sync_all_signal.emit(self.nao))
        self.addAction(self.sync_all_action)

        self.update()

    @property
    def name(self) -> str:
        return self._nao.name

    @property
    def nao(self):
        return self._nao

    @property
    def connection(self) -> Optional[NaoConnection]:
        return self._connection

    @connection.setter
    def connection(self, nao_connection: Optional[NaoConnection]):
        self._connection = nao_connection

    def retranslateUi(self, window):
        super().retranslateUi(window)
        self.nao_backend.clicked.connect(self.handle_backend_click)
        self.nao_frontend.clicked.connect(self.handle_frontend_click)
        self.nao_status.clicked.connect(self.handle_nao_status_click)
        self.selectedCheckbox.stateChanged.connect(lambda selected: self.selected_nao.emit(self.nao, selected))

    def resizeEvent(self, event):
        super().resizeEvent(event)

    def handle_backend_click(self):
        self.nao.send_monitor(NaoMonitorAction.BACKEND, not self.nao.config.backendRunning)
        self._parent.focusNao(self)

    def handle_frontend_click(self):
        self.nao.send_monitor(NaoMonitorAction.FRONTEND, not self.nao.config.frontendRunning)
        self._parent.focusNao(self)

    def handle_nao_status_click(self):
        debugger = BembelDbugFactory().get()
        if self.connection is None:
            debugger.connect_to_nao(self._nao.name)
            self._attemptConnection = True
        else:
            debugger.disconnect_from_nao(self._nao.name)
            self._attemptConnecion = False
        self._parent.focusNao(self)

    def setFocus(self):
        self.setFrameShape(QFrame.Panel)
        #self.selectedCheckbox.setCheckState(Qt.Checked)

    def removeFocus(self):
        self.setFrameShape(QFrame.NoFrame)
        #self.selectedCheckbox.setCheckState(Qt.Unchecked)

    def update(self):

        config = self._nao.config
        self.nao_title.setTitle(self._nao.name.capitalize())
        self.nao_name.setText(self._nao.name.capitalize())
        self.nao_role.setText(config.role)
        self.nao_team_id.setText("{0} - {1}".format(config.teamNr, config.jerseyNr))

        essid_text = config.essid

        # Last Seen
        last_seen_seconds = time.time() - self._nao.last_seen
        self.nao_last_seen.setText("{0:.2f}s".format(last_seen_seconds))

        # Battery
        battery_text = "{0:.0f}%".format(config.battery * 100)
        if config.batteryCharging:
            battery_text += " (plugged in)"
        if config.battery < 0:
            battery_text = "Unknown"

        self.nao_battery_level.setText(battery_text)

        # Wlan
        if self.nao.config.wlan is None or self.nao.config.wlan != "":
            self.ssh_action_wlan.setEnabled(True)
            self.nao_wlan.setText(config.wlan)
            essid_text = "{0} @ {1} dBm".format(config.essid, config.signalStrength)
        else:
            self.nao_wlan.setText("None")
            self.ssh_action_wlan.setEnabled(False)
            essid_text = "None"

        self.nao_essid.setText(essid_text)

        # Lan
        if self.nao.config.lan is None or self.nao.config.lan != "":
            self.nao_eth.setText(config.lan)
            self.nao_eth.setEnabled(self.nao.config.lan_link)
            self.ssh_action_lan.setEnabled(True)
        else:
            self.nao_eth.setText("None")
            self.ssh_action_lan.setEnabled(False)

        # Frontend
        if config.frontendRunning:
            self.nao_frontend.setStyleSheet("background-color: green")
        else:
            self.nao_frontend.setStyleSheet("background-color: rgb(128, 128, 128)")

        # Backend
        if config.backendRunning:
            self.nao_backend.setStyleSheet("background-color: green")
        else:
            self.nao_backend.setStyleSheet("background-color: rgb(128, 128, 128)")

        if not self.nao.config.frontendRunning:
            self.nao_status.setEnabled(False)
            self.nao_status.setText("No Frontend")
            self.nao_status.setStyleSheet(NaoStatusColors.STATUS_BUSY.value)
        elif self.connection is not None and self.connection.is_connected():
            self.nao_status.setEnabled(True)
            self.nao_status.setText("Disconnect")
            self.nao_status.setStyleSheet(NaoStatusColors.STATUS_CONNECTED.value)
            self._attemptConnection = False
        elif self._attemptConnection:
            self.nao_status.setEnabled(False)
            self.nao_status.setText("Connecting...")
            self.nao_status.setStyleSheet(NaoStatusColors.STATUS_CONNECTING.value)
        elif not self.nao.is_debuggable():
            self.nao_status.setEnabled(False)
            self.nao_status.setText("Nao Busy")
            self.nao_status.setStyleSheet(NaoStatusColors.STATUS_BUSY.value)
        else:
            self.nao_status.setStyleSheet(NaoStatusColors.STATUS_DISCONNECTED.value)
            self.nao_status.setText("Connect")
            self.nao_status.setEnabled(True)
