import socket

from bembelapi.config import NaoEvent
from bembelapi.connection_manager import NaoConnectionManager
from bembelapi.naomodule import NaoModule

class SaveBlackboardsListener(NaoModule):

    def __init__(self, connection_manager: NaoConnectionManager):
        super().__init__()
        self.connection_manager =  connection_manager

    def setup(self, parent):
        super().setup(parent)
        parent.add_listener("message_save_blackboards_ok", self)
        parent.add_listener("message_save_blackboards_error", self)

    def handle_message_save_blackboards_ok(self, message: dict, client: (str, int), _socket: socket):
        self.parent.emit(NaoEvent.NAO_SAVE_BLACKBOARDS_OK)

    def handle_message_save_blackboards_error(self, message: dict, client: (str, int), _socket: socket):
        self.parent.emit(NaoEvent.NAO_SAVE_BLACKBOARDS_ERROR)
