import json
import socket
import time

from bembelapi.config import NaoEvent, KEEPALIVE_INTERVAL
from bembelapi.connection_manager import NaoConnectionManager
from bembelapi.nao import RobotHeartbeat, NaoInfo
from bembelapi.naomodule import NaoModule
from bembelapi.server import NaoDebugUdpServer


class NaoHeartbeat(NaoModule):
    _packet = {
        "debugv2": {
            "msg_type": "keepalive"
        }
    }

    def __init__(self, server: NaoDebugUdpServer, connection_manager: NaoConnectionManager):
        super().__init__()
        self.connection_manager = connection_manager
        RobotHeartbeat.keepalive_interval = KEEPALIVE_INTERVAL
        self.socket = server.socket
        self._packet = str.encode(json.dumps(self._packet))

    def setup(self, parent):
        super().setup(parent)

    def tick(self):
        nao_connection = self.connection_manager.connection

        if nao_connection is None:
            return

        # disconnect if frontend has been stopped
        if nao_connection.is_connected() and not nao_connection.is_frontend_running():
            self.parent.emit(NaoEvent.NAO_DISCONNECT, nao_connection.name)
            return

        heartbeat = nao_connection.get_heartbeat()

        if heartbeat.is_valid():
            return

        is_new_connection = not nao_connection.is_connected()

        nao = nao_connection.info

        try:
            self.socket.sendto(self._packet, nao.debug_address)
        except socket.timeout:
            return
        except Exception as e:
            return

        heartbeat = RobotHeartbeat(time.time(), not nao.free)
        nao_connection.update_heartbeat(heartbeat)

        if is_new_connection and nao_connection.is_connected():
            self.parent.emit(NaoEvent.NAO_CONNECTED, nao_connection)