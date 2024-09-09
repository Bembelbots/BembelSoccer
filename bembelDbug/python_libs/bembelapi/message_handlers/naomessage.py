import json
import socket

from bembelapi.events import EventEmitter
from bembelapi.config import NaoEvent
from bembelapi.message_handler import MessageHandler, MessageHandlerEvent
from bembelapi.nao import RobotConfig


class NaoConfigEvent(MessageHandlerEvent):
    def __init__(self, data: RobotConfig, client_address: (str, int), _socket: socket):
        super().__init__(client_address, _socket)
        self.data = data

    def dispatch(self, event_emitter: EventEmitter):
        event_emitter.emit(NaoEvent.NAO_CONFIG, self.data, self.client_address, self.socket)

class NaoMessageEvent(MessageHandlerEvent):
    def __init__(self, data: dict, client_address: (str, int), _socket: socket):
        super().__init__(client_address, _socket)
        self.data = data

    def dispatch(self, event_emitter: EventEmitter):
        message = self.data.get("debugv2")
        if message is None or message.get("msg_type") is None:
            return
        nao_event = "message_" + message["msg_type"]
        del message["msg_type"]
        event_emitter.emit(nao_event, message, self.client_address, self.socket)


class NaoMessageHandler(MessageHandler):
    def handle(self):
        data, _socket = self.request

        try:
            message = json.loads(data.decode("utf-8"))
            self._emit(NaoMessageEvent(message, self.client_address, _socket))
        except:
            config = RobotConfig.parseData(data)
            self._emit(NaoConfigEvent(config, self.client_address, _socket))

        #except Exception as e:
        #    print("failed to load message %s" % data)
        #    print(str(e))
