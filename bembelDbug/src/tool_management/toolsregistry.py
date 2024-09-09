from enum import Enum
from typing import Optional

from PyQt5.QtCore import pyqtSignal, QSettings
from PyQt5.QtGui import QCloseEvent
from PyQt5.QtWidgets import QPushButton
from PyQt5.QtWidgets import QSizePolicy
from PyQt5.QtWidgets import QWidget
from PyQt5.QtWidgets import QMdiSubWindow
from bembelapi.naoconnection import NaoConnection

from config import PROJECT_NAME
from widgets.nao.widget import NaoWidget


class ToolTypes(Enum):
    NORMAL = 0
    MONITOR = 1
    CONNECTION = 2


class Tool(QWidget):
    _type = ToolTypes.NORMAL
    closing = pyqtSignal(QCloseEvent)

    def __init__(self, name: str, button_text: str):
        super().__init__()
        self.name = name
        self.button_text = button_text
        self._qsettings = QSettings()
        self._nao_widget = None  # type: Optional[NaoWidget]
        self._connection = None  # type: Optional[NaoConnection]

    @property
    def type(self):
        return self._type.value

    @property
    def nao_widget(self):
        return self._nao_widget

    @property
    def connection(self):
        return self._connection

    @nao_widget.setter
    def nao_widget(self, nao: NaoWidget):
        self._nao_widget = nao
        self._connection = self._nao_widget.connection

    def setup(self):
        self.mdiMode = isinstance(self.parentWidget(), QMdiSubWindow)
        self._geometryCfg = "mdiGeometry" if self.mdiMode else "windowGeometry"
        self._windowWidget = self.parentWidget() if self.mdiMode else self
        
        self.setWindowTitle("{0} ({1}) : {2}"
                            .format(PROJECT_NAME,
                                    self._nao_widget.name.capitalize(),
                                    self.button_text.replace('\n', ' ').capitalize()))

        geometry = self.getConfig(self._geometryCfg)
        if geometry and self.__cfgRememberGeometry():
            self._windowWidget.restoreGeometry(geometry)

    def shutdown(self):
        pass

    def closeEvent(self, event: QCloseEvent):
        if self.__cfgRememberGeometry():
            self.setConfig(self._geometryCfg, self._windowWidget.saveGeometry())
        self.closing.emit(event)
        self._windowWidget.close()
        event.accept()

    def show(self):
        self.setup()
        super().show()

    def setConfig(self, varname, value):
        self._qsettings.setValue(self.__cfgPrefix(varname), value)

    def getConfig(self, varname):
        value = self._qsettings.value(self.__cfgPrefix(varname))
        return value
    
    def __cfgPrefix(self, varname):
        return "{}/{}".format(self.name, varname)

    def __cfgRememberGeometry(self):
        return bool(int(self._qsettings.value("rememberGeometry")))

    @classmethod
    def register(cls):
        ToolsRegistry.add_tool(cls.name, cls.button_text, cls)


class MonitorTool(Tool):
    _type = ToolTypes.MONITOR


class ConnectionTool(Tool):
    _type = ToolTypes.CONNECTION


class ActivateToolButton(QPushButton):
    def __init__(self, *__args):
        super().__init__(*__args)
        self._nao_widget = None  # type: Optional[NaoWidget]

    @property
    def nao_widget(self):
        return self._nao_widget

    @nao_widget.setter
    def nao_widget(self, nao: NaoWidget):
        self._nao_widget = nao


class ToolsRegistry:
    """
    Registry for all Tools (aka Tool Suites).

    Provides handlers for opening and closing a Tool
    """
    _registered_tools = {}
    _tool_buttons = {}
    _tool_objects = {}

    @classmethod
    def get(cls, name) -> Tool.__class__:
        return cls._registered_tools[name][1]

    @classmethod
    def tools(cls, tool_type: ToolTypes):
        tools = [key for key, (_, tool) in cls._registered_tools.items() if tool.type != tool_type.value]
        tools.sort()
        return tools

    @classmethod
    def buttons(cls, mdi) -> list:
        """
        Grabs all registered tool_management and returns open buttons for them in a sorted list.

        :return: list of tool buttons
        """
        buttons = []

        registered_tools = list(cls._registered_tools.items())
        registered_tools.sort(key=lambda x: x[0])

        for name, (text, cls_) in registered_tools:
            buttons.append(cls._create_button(name, text, mdi, cls_))

        return buttons

    @classmethod
    def open_windows(cls):
        """
        Returns a list of currently open tool windows.
        :return: list of open tool windows
        """
        return list(cls._tool_objects.values())

    @classmethod
    def _create_button(cls, name, text, mdi, cls_) -> ActivateToolButton:
        """
        Returns a list of currently open tool windows.
        :return: list of open tool windows
        """
        button = ActivateToolButton()
        button.setText(text)
        button.setMinimumSize(150, 50)
        button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        button.clicked.connect(cls.handle_tool_click(name, mdi))
        cls._tool_buttons[name] = button
        return button

    @classmethod
    def handle_tool_close(cls, name):
        """
        Returns a list of currently open tool windows.
        :return: list of open tool windows
        """

        def handler(event):
            event.accept()
            cls._tool_objects[name].shutdown()
            del cls._tool_objects[name]

        return handler

    @classmethod
    def launch_tool(cls, name, mdi=None):
        """
        Lauches specified tool.
        :param name: string name of tool to be launched
        :param mdi: parent mdi widget if in single window mode
        """
        obj = cls._tool_objects.get(name)  # type: QWidget
        if obj is not None:
            obj.raise_()
            return

        if not name in cls._registered_tools:
            print("Error: cannot launch tool '{}', tool is not registered".format(name))
            return

        obj = cls._registered_tools[name][1]()
        obj.closing.connect(cls.handle_tool_close(name))
        cls._tool_objects[name] = obj
        obj.nao_widget = cls._tool_buttons[name].nao_widget

        if mdi is not None:
            mdi.addSubWindow(obj)
        obj.show()

    @classmethod
    def handle_tool_click(cls, name, mdi):
        """
        Returns a handler for opening the specified tool.
        :param name: string name describing clicked tool
        :param tool_class:
        :return: handler function opening the tool
        """

        def handler():
            cls.launch_tool(name, mdi)
        
        return handler

    @classmethod
    def add_tool(cls, name: str, button_text: str, tool_class):
        """
        Returns a handler for opening the specified tool.
        :param name: string name describing clicked tool
        :param tool_class:
        :return: handler function opening the tool
        """
        cls._registered_tools[name] = (button_text, tool_class)
