import socket
import socketserver

from bembelapi.events import EventEmitter


class MessageHandlerEvent:
    def __init__(self, client_address: (str, int), _socket: socket):
        self.type = self.__class__.__name__.lower()
        self.client_address = client_address
        self.socket = _socket
        self.data = None

    def dispatch(self, event_emitter: EventEmitter):
        pass


class MessageHandler(socketserver.BaseRequestHandler):
    def _emit(self, event: MessageHandlerEvent):
        self.server.queue.put_nowait(event)
