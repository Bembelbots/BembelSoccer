import socket

from bembelapi.config import NaoEvent
from bembelapi.connection_manager import NaoConnectionManager
from bembelapi.naomodule import NaoModule


class NaoSymbolsListener(NaoModule):
    def __init__(self, connection_manager: NaoConnectionManager):
        super().__init__()
        self.connection_manager = connection_manager

    def setup(self, parent):
        super().setup(parent)
        parent.add_listener("message_symbol_names", self)
        parent.add_listener("message_symbol_values", self)

    def handle_message_symbol_names(self, message: dict, client: (str, int), _socket: socket):
        nao_connection = self.connection_manager.connection
        if not nao_connection:
            return
        nao_connection.symbol_names = message
        self.parent.emit(NaoEvent.NAO_SYMBOL_NAMES, message, nao_connection)

    def handle_message_symbol_values(self, message: dict, client: (str, int), _socket: socket):
        nao_connection = self.connection_manager.connection
        if not nao_connection:
            return

        self.parent.emit(NaoEvent.NAO_SYMBOL_VALUES, message, nao_connection)

    def tick(self):
        pass
