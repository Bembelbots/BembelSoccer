import sys
from typing import Optional

from bembelapi.config import NaoEvent
from bembelapi.nao import NaoInfo
from bembelapi.naoconnection import NaoConnection

from debugger import BembelDbugFactory
from tool_management.toolsregistry import ToolsRegistry
from widgets.nao.widget import NaoWidget


class ToolLauncher:
    def __init__(self, nao_name: str, tool_name: str):
        self.window = None
        self.tool_name = tool_name
        self.nao_name = nao_name
        self.nao_widget = None  # type: Optional[NaoWidget]
        self._attempt_connect = False

        self.debugger = BembelDbugFactory().get()
        self.debugger.add_listener(NaoEvent.NAO_FOUND, self.handle_nao_found)
        self.debugger.add_listener(NaoEvent.NAO_CONNECTED, self.handle_nao_connected)
        self.debugger.add_listener(NaoEvent.NAO_UPDATE, self.handle_nao_update)
        self.debugger.add_listener(NaoEvent.NAO_DISCONNECTED, self.handle_nao_disconnected)

    def handle_nao_found(self, nao: NaoInfo):
        if nao.name != self.nao_name:
            return
        self.nao_widget = NaoWidget(nao)
        print(str.format("{0}: found", self.nao_name))

    def handle_nao_update(self, nao: NaoInfo):
        if nao.name != self.nao_name or not nao.is_debuggable():
            return

        if not self._attempt_connect:
            self._attempt_connect = True
            self.debugger.connect_to_nao(self.nao_name)
            print(str.format("{0}: connecting", self.nao_name))

    def handle_nao_connected(self, connection: NaoConnection):
        if connection.name != self.nao_name:
            return
        self._attempt_connect = False
        print(str.format("{0}: connected", self.nao_name))
        self.nao_widget.connection = connection
        self.window = ToolsRegistry.get(self.tool_name)()
        self.window.nao_widget = self.nao_widget
        self.window.show()

    def handle_nao_disconnected(self, connection: NaoConnection):
        if connection.name != self.nao_name:
            return
        sys.exit()
