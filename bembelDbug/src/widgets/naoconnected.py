from PyQt5 import QtWidgets
from PyQt5.QtGui import QCloseEvent
from PyQt5.QtWidgets import QScrollArea
from PyQt5.QtWidgets import QWidget, QMdiArea
from PyQt5 import QtCore
from PyQt5.QtCore import QSettings
from debugger import BembelDbugFactory
from layouts.flowlayout import FlowLayout
from tool_management.toolsregistry import ToolsRegistry
from widgets.nao.widget import NaoWidget


class NaoConnectedWidget(QWidget):
    def __init__(self, nao_widget: NaoWidget, mdi: QMdiArea):
        super().__init__()
        self.mdi = mdi
        self.nao_widget = nao_widget
        self.setupUi(self)

    # noinspection PyAttributeOutsideInit
    def setupUi(self, window):
        self.verticalLayout = QtWidgets.QVBoxLayout(window)

        self.scrollArea = QScrollArea(window)
        self.scrollArea.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.scrollArea.setWidgetResizable(True)
        self.scrollArea.setObjectName("scrollArea")
        self.scrollAreaWidgetContents = QtWidgets.QWidget()
        self.scrollAreaWidgetContents.setFocusPolicy(QtCore.Qt.NoFocus)
        self.scrollAreaWidgetContents.setObjectName("scrollAreaWidgetContents")
        self.scrollArea.setWidget(self.scrollAreaWidgetContents)

        self.tools_layout = FlowLayout(self.scrollAreaWidgetContents)

        for widget in ToolsRegistry.buttons(self.mdi):
            widget.nao_widget = self.nao_widget
            self.tools_layout.addWidget(widget)

        self.verticalLayout.addWidget(self.scrollArea)

        tools = QSettings().value("autostartTools")
        if tools:
            print("Launching tools: ", tools)
            for tool in tools:
                ToolsRegistry.launch_tool(tool, self.mdi)

    def closeEvent(self, event: QCloseEvent):
        openTools = []
        for window in ToolsRegistry.open_windows():
            openTools.append(window.name)
            window.close()
        
        s = QSettings()
        if int(s.value("rememberTools")):
            s.setValue("autostartTools", openTools)
            print("Saving opened tools:", openTools)

        debugger = BembelDbugFactory().get()
        debugger.disconnect_from_nao(self.nao_widget.name)
        event.accept()
