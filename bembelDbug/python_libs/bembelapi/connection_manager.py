import threading
from typing import Optional

from bembelapi.config import NaoEvent
from bembelapi.events import EventEmitter
from bembelapi.nao import NaoInfo
from bembelapi.naoconnection import NaoConnection
from bembelapi.naoinfolist import NaoInfoList
from bembelapi.server import NaoDebugUdpServer


class NaoConnectionManager:
    def __init__(self, parent: EventEmitter, server: NaoDebugUdpServer):
        super().__init__()

        self._server = server
        self._parent = parent

        self._nao_list = NaoInfoList(parent)
        self._nao_list_lock = threading.Lock()  # type: threading.Lock

        self.connection_lock = threading.Lock()  # type: threading.Lock

        self.attempt_connect = {}
        self.connections = {}

        parent.add_listener(NaoEvent.NAO_CONNECT, self)
        parent.add_listener(NaoEvent.NAO_LOST, self)
        parent.add_listener(NaoEvent.NAO_DISCONNECT, self)
        parent.add_listener(NaoEvent.NAO_DISCONNECTED, self)

    @property
    def nao_list(self) -> NaoInfoList:
        with self._nao_list_lock:
            return self._nao_list

    def peek_nao_list(self) -> NaoInfoList:
        retval = None

        if self._nao_list_lock.acquire(False):
            retval = self._nao_list
            self._nao_list_lock.release()

        return retval

    @property
    def connection(self) -> Optional[NaoConnection]:
        with self.connection_lock:
            connections = list(self.connections.values())
            if len(connections) == 0:
                return None
            return connections[0]

    def handle_nao_connect(self, name: str):
        nao_info = self._nao_list.get(name)

        if nao_info is None or not nao_info.is_debuggable():
            self.attempt_connect[name] = False
            return

        connection = self.connections.get(name)  # type: NaoConnection

        if connection and connection.info == nao_info:
            return

        current_connections = list(self.connections.values())

        # allow only one connection for now
        if len(current_connections) > 0:
            current_connections[0].stop_recv_images()
            self._parent.emit(NaoEvent.NAO_DISCONNECTED, current_connections[0])

        self.connections[name] = NaoConnection(nao_info, self._server.socket, self._parent)

    def handle_nao_disconnect(self, name: str):
        if self.connections.get(name):
            self._parent.emit(NaoEvent.NAO_DISCONNECTED, self.connections[name])

    def handle_nao_disconnected(self, nao: NaoConnection):
        if self.connections.get(nao.info.name):
            del self.connections[nao.info.name]

    def handle_nao_lost(self, nao_info: NaoInfo):
        connection = self.connections.get(nao_info.name)
        if connection:
            self._parent.emit(NaoEvent.NAO_DISCONNECTED, connection)
