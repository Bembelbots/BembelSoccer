import json
import socket

from bembelapi.config import NaoEvent
from bembelapi.events import EventEmitter
from bembelapi.message_handler import MessageHandler, MessageHandlerEvent
from bembelapi.modules.spl import SPLStandardMessage


class SplMessageEvent(MessageHandlerEvent):
    def __init__(self, data: SPLStandardMessage, client_address: (str, int), _socket: socket):
        super().__init__(client_address, _socket)
        self.data = data

    def dispatch(self, event_emitter: EventEmitter):
        event_emitter.emit(NaoEvent.SPL_STANDARD_MESSAGE, self.data, self.client_address, self.socket)


class SplMessageHandler(MessageHandler):
    def handle(self):
        data, _socket = self.request

        try:
            message = SPLStandardMessage(data)
            self._emit(SplMessageEvent(message, self.client_address, _socket))
        except Exception as e:
            print("failed to load message %s" % data)
            print(str(e))
