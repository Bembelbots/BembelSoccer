import os
from typing import Optional
import subprocess
from os.path import dirname

from PyQt5.QtWidgets import QMainWindow, QMdiArea, QInputDialog, QLineEdit
from PyQt5.QtCore import QThread, Qt, QSettings
from PyQt5.QtGui import QCloseEvent

from config import NaoStatusColors, PROJECT_NAME
from debugger import BembelDbugFactory
from ui.mainwindow_ui import Ui_MainWindow
from widgets.nao.widget import NaoWidget
from widgets.nao_calibration.widget import NaoCalibrationWidget
from widgets.nao_config.widget import NaoConfigWidget
from widgets.nao_list.widget import NaoListWidget
from widgets.naoconnected import NaoConnectedWidget
from widgets.nao_sync.widget import NaoSyncWidget
from bembelapi.nao import NaoInfo


file_path = dirname(__file__)
identity_file = ""


class MainWindowWrapper(QMdiArea):

    def __init__(self):
        super().__init__()
        self.main_window = MainWindow(self)
        # Remove close-button from main window, you don't want to close that one on accident
        self.addSubWindow(self.main_window, Qt.CustomizeWindowHint | Qt.WindowTitleHint | Qt.WindowMinMaxButtonsHint)

        self.setWindowTitle(PROJECT_NAME)

        s = QSettings()
        root_geometry = s.value("mdiWindowGeometry")
        mdi_geometry = s.value("mdiGeometry")
        remember = s.value("rememberGeometry")
        if remember and bool(int(remember)):
            if root_geometry:
                self.restoreGeometry(root_geometry)
            if mdi_geometry:
                self.main_window.parentWidget().restoreGeometry(mdi_geometry)

    def closeEvent(self, event: QCloseEvent):
        s = QSettings()
        remember = s.value("rememberGeometry")
        if remember and bool(int(remember)):
            s.setValue("mdiWindowGeometry", self.saveGeometry())
            s.setValue("mdiGeometry", self.main_window.parentWidget().saveGeometry())
        event.accept()


class MainWindow(QMainWindow):
    """
    Main window class handling UI events and updating it accordingly.
    """
    def __init__(self, mdi=None):
        super().__init__()
        self.settings = QSettings()

        self.mdi = mdi
        self.selected_nao = None  # type: Optional[NaoWidget]
        self.nao_config_widget = NaoConfigWidget()
        self.nao_calibration_widget = NaoCalibrationWidget()
        self.nao_sync_widget = NaoSyncWidget()
        self.tab_connected = None  # type: Optional[NaoConnectedWidget]
        self.attempt_connect = False

        self.nao_list = NaoListWidget()

        # listen for events
        self.nao_list.nao_focused.connect(self.handle_nao_focused)
        self.nao_list.nao_lost.connect(self.handle_nao_lost)
        self.nao_list.nao_updated.connect(self.handle_nao_updated)
        self.nao_list.nao_connected.connect(self.handle_nao_connected)
        self.nao_list.nao_disconnected.connect(self.handle_nao_disconnected)        
        self.nao_list.sync_all_signal.connect(self.handle_sync_all_context_menu)
        self.nao_list.sync_frontend_signal.connect(self.handle_sync_frontend_context_menu)

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.setWindowTitle(PROJECT_NAME)

        # register button clicks
        self.ui.nao_ssh_eth.clicked.connect(self.handle_ssh_eth)
        self.ui.nao_ssh_wlan.clicked.connect(self.handle_ssh_wlan)

        self.ui.naolist.layout().addWidget(self.nao_list)
        self.ui.tab_config.layout().addWidget(self.nao_config_widget)
        self.ui.tab_calibration.layout().addWidget(self.nao_calibration_widget)
        self.ui.sync_widget_container.layout().addWidget(self.nao_sync_widget)

        self.ui.actionssh_all_eth0.triggered.connect(self.handle_ssh_all_eth)
        self.ui.actionssh_all_wlan0.triggered.connect(self.handle_ssh_all_wlan)        
        self.ui.action_remember_tools.triggered.connect(self.handle_remember_tools)
        self.ui.action_remember_geometry.triggered.connect(self.handle_remember_geometry)
        self.ui.action_clear_autostart.triggered.connect(self.handle_clear_autostart)
        self.ui.action_mdi_mode.triggered.connect(self.handle_mdi_mode)
        
        self.ui.actionadd_nao.triggered.connect(self.handle_add_nao)
        self.ui.actionclear_added_naos.triggered.connect(self.handle_clear_naos)

        rememberTools = self.__getSetting("rememberTools")
        self.ui.action_remember_tools.setChecked(rememberTools)
        
        rememberGeometry = self.__getSetting("rememberGeometry")
        self.ui.action_remember_geometry.setChecked(rememberGeometry)

        self.ui.action_mdi_mode.setChecked(self.__getSetting("forceMdiMode", 0))
        
        geometry = self.settings.value("windowGeometry")
        if geometry and rememberGeometry:
            self.restoreGeometry(geometry)
        
    def closeEvent(self, event: QCloseEvent):
        if self.__getSetting("rememberGeometry"):
            self.settings.setValue("windowGeometry", self.saveGeometry())
        event.accept()

    def handle_sync_frontend_context_menu(self, info):
        if info != self.selected_nao.nao:
            print("right click context menu press on not selected nao!")
            return
        self.nao_sync_widget.syncFrontend()

    def handle_sync_all_context_menu(self, info):
        if info != self.selected_nao.nao:
            print("right click context menu press on not selected nao!")
            return
        self.nao_sync_widget.syncAll()

    def handle_ssh_eth(self):
        if self.selected_nao is None:
            return
        self.selected_nao.nao.do_ssh("eth")

    def handle_ssh_wlan(self):
        if self.selected_nao is None:
            return
        self.selected_nao.nao.do_ssh("wlan")

    def handle_ssh_all_eth(self):
        self.__do_ssh_all("eth")

    def handle_ssh_all_wlan(self):
        self.__do_ssh_all("wlan")

    def handle_remember_tools(self):
        state = not self.__getSetting("rememberTools")
        self.ui.action_remember_tools.setChecked(state)
        self.settings.setValue("rememberTools", int(state))
    
    def handle_remember_geometry(self):
        state = not self.__getSetting("rememberGeometry")
        self.ui.action_remember_geometry.setChecked(state)
        self.settings.setValue("rememberGeometry", int(state))
     
    def handle_clear_autostart(self):
        self.settings.setValue("autostartTools", [])

    def handle_mdi_mode(self):
        state = not self.__getSetting("forceMdiMode")
        self.ui.action_remember_tools.setChecked(state)
        self.settings.setValue("forceMdiMode", int(state))

    def handle_nao_updated(self, nao_widget: NaoWidget):
        if self.selected_nao != nao_widget:
            return
        config = self.selected_nao.nao.config
        self.ui.nao_ssh_wlan.setEnabled(config.wlan != "")
        self.ui.nao_ssh_eth.setEnabled(config.lan_link and config.lan != "")
        self.nao_sync_widget.setActiveRobot(self.selected_nao.nao)

    def handle_nao_lost(self, nao_widget: NaoWidget):
        if self.selected_nao != nao_widget:
            return

        self.ui.nao_edit_tabs.setCurrentIndex(0)
        self.ui.nao_edit_tabs.setEnabled(False)
        self.ui.nao_ssh_eth.setEnabled(False)
        self.ui.nao_ssh_wlan.setEnabled(False)

        self.nao_config_widget.set_nao(None)
        self.nao_calibration_widget.set_nao(None)
        self.close_connected_window(nao_widget)
        self.selected_nao = None

    def handle_nao_focused(self, nao_widget: NaoWidget):
        if self.selected_nao == nao_widget:
            return

        self.selected_nao = nao_widget
        self.nao_config_widget.set_nao(nao_widget.nao)
        self.nao_calibration_widget.set_nao(nao_widget.nao)

        self.ui.nao_edit_tabs.setCurrentIndex(0)
        self.ui.nao_edit_tabs.setEnabled(True)

        self.handle_nao_updated(nao_widget)
        self.update_tabs(nao_widget)

    def handle_add_nao(self):
        ip, okPressed = QInputDialog.getText(self, "Add Nao","IP Address", QLineEdit.Normal, "")
        if okPressed and ip != '':
            debugger = BembelDbugFactory().get()
            nao_debug = debugger.nao_debug
            nao_debug.add_nao_ip(ip)

    def handle_clear_naos(self):
        debugger = BembelDbugFactory().get()
        nao_debug = debugger.nao_debug
        nao_debug.clear_nao_ips()

    def handle_connect_btn(self):
        debugger = BembelDbugFactory().get()

        if self.selected_nao.connection is None:
            self.attempt_connect = True
            debugger.connect_to_nao(self.selected_nao.name)
        else:
            self.attempt_connect = False
            debugger.disconnect_from_nao(self.selected_nao.name)

    def handle_nao_connected(self, nao_widget: NaoWidget):
        if self.selected_nao != nao_widget:
            return
        self.attempt_connect = False
        self.tab_connected = NaoConnectedWidget(nao_widget, self.mdi)
        self.update_tabs(nao_widget)

    def handle_nao_disconnected(self, nao_widget: NaoWidget):
        self.close_connected_window(nao_widget)

    def close_connected_window(self, nao_widget: NaoWidget):
        if self.tab_connected is None or self.tab_connected.nao_widget != nao_widget:
            return

        self.attempt_connect = False
        self.tab_connected.close()

        self.tab_connected = None
        self.update_tabs(nao_widget)


    def update_tabs(self, nao_widget):
        if self.ui.nao_edit_tabs.tabText(0) == "Tools":
            self.ui.nao_edit_tabs.removeTab(0)

        if self.tab_connected and self.tab_connected.nao_widget == nao_widget:
            self.ui.nao_edit_tabs.insertTab(0, self.tab_connected, "Tools")
            self.ui.nao_edit_tabs.setCurrentIndex(0)

    def __do_ssh_all(self, type: str):
        ip_list = []
        for name, widget in self.nao_list.naos.items():
            nao = widget.nao
            if type == "wlan":
                ip_list.append(nao.config.wlan)
            elif type == "eth" and nao.config.lan_link:
                ip_list.append(nao.config.lan)
            else:
                pass

        args = " ".join(["nao@%s" % ip for ip in ip_list])

        subprocess.call("cssh -o \"-i %s\" %s &" % (identity_file, args), shell=True)

    def __getSetting(self, name, default=1):
        s = self.settings.value(name)
        if s is None:
            s = default
            self.settings.setValue(name, s)
        return bool(int(s))

